// General Libraries
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "csv.hpp"

// RocksDB Libraries
#include <rocksdb/db.h>
#include <rocksdb/options.h>


// Namespaces
using namespace std;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::DBOptions;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Slice;

DB* create_kvs(const string& csv_file_path, const string& db_path) {
    csv::CSVReader reader(csv_file_path);
    csv::CSVRow row;

    vector<string> header = reader.get_col_names();

    #if 0
        int col_no = 0;
        for (csv::CSVRow& row : reader) {
            col_no = 0;
            cout << "ID: " << row["col_name"] << endl;
            for (csv::CSVField& field : row) {
                cout << header[col_no]  << ": " << field.get<string>() << endl;
                col_no++;
            }
            break;
        }
    #endif

    DB* db;

    Options options;
    options.create_if_missing = true;
    // Optimization: Increase write buffer for faster bulk loading
    options.write_buffer_size = 64 * 1024 * 1024; // 64MB
    // Optimization: Disable WAL during bulk load for speed
    options.wal_bytes_per_sync = 0;
    
    Status status = DB::Open(options, db_path, &db);
    if (!status.ok()) {
        cerr << "Unable to open/create database: " << status.ToString() << endl;
        return nullptr;
    }
    WriteBatch batch;
    WriteOptions write_options;
    write_options.disableWAL = true;
    
    size_t batch_size = 0;
    const size_t max_batch_size = 10 * 1024 * 1024; // 10MB batches
    
    for (csv::CSVRow& row : reader) {
        string id_value = row["id"].get<string>();
        size_t col_no = 0;
        
        for (csv::CSVField& field : row) {
            // Optimization: Use reserve to avoid string reallocation
            string key;
            key.reserve(id_value.size() + header[col_no].size() + 1);
            key = id_value + "_" + header[col_no];
            
            string value = field.get<string>();
            batch.Put(key, value);
            batch_size += key.size() + value.size();
            col_no++;
        }
        
        if (batch_size >= max_batch_size) {
            status = db->Write(write_options, &batch);
            if (!status.ok()) {
                cerr << "Error writing batch to database: " << status.ToString() << endl;
                delete db;
                return nullptr;
            }
            batch.Clear();
            batch_size = 0;
        }
    }
    
    // Write remaining batch
    if (batch.Count() > 0) {
        status = db->Write(write_options, &batch);
        if (!status.ok()) {
            cerr << "Error writing batch to database: " << status.ToString() << endl;
            delete db;
            return nullptr;
        }
    }

    return db;
}

vector<string> multi_get(DB* db, const vector<string>& keys) {
    vector<string> values;
    values.reserve(keys.size());

    vector<Slice> key_slices;
    key_slices.reserve(keys.size());
    for (const auto& key : keys) {
        key_slices.push_back(Slice(key));
    }
    
    vector<string> result_values;
    result_values.reserve(keys.size());
    vector<Status> statuses = db->MultiGet(ReadOptions(), key_slices, &result_values);
    
    for (size_t i = 0; i < statuses.size(); i++) {
        if (statuses[i].ok()) {
            values.push_back(result_values[i]);
        } else {
            values.push_back("");
        }
    }

    return values;
}
vector<string> iterate_over_range(DB* db, const string& start_key, const string& end_key) {
    vector<string> result;
    result.reserve(100); // Optimization: Reserve space for expected results
    
    ReadOptions read_options;
    // Optimization: Use fill_cache=false if iterating once over large range
    read_options.fill_cache = false;
    
    rocksdb::Iterator* it = db->NewIterator(read_options);
    it->Seek(start_key);
    
    // Optimization: Pre-compute search string
    static const string display_name_suffix = "_display_name";
    
    while (it->Valid()) {
        Slice current_key = it->key();
        
        if (current_key.compare(end_key) > 0) {
            break;
        }
        
        if (current_key.size() > display_name_suffix.size() &&
            current_key.ends_with(display_name_suffix)) {
            result.push_back(it->value().ToString());
        }
        
        it->Next();
    }
    
    if (!it->status().ok()) {
        cerr << "Iterator error: " << it->status().ToString() << endl;
    }
    
    delete it;
    return result;
}

Status delete_key(DB* db, const string& key) {
    Status s;
    WriteOptions write_options;
    s = db->Delete(write_options, key);

    return s;
}
