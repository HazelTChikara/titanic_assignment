// Test the assignment-nosql.cc file

#include "assignment-nosql.hpp"
#include <iostream>
#include <iomanip>

// Namespaces
using namespace std;
using namespace rocksdb;


int main() {
    int total_score = 0;
    int max_score = 100;

    cout << "========================================" << endl;
    cout << "    NoSQL Assignment Grading Rubric" << endl;
    cout << "========================================" << endl << endl;

    // 0. set up the paths
    // Do not change these paths
    const std::string csv_file_path = "subreddits.csv";
    const std::string db_path = "./test/subreddits_rdb";

    // Test 1: Create and Load KVS from CSV (25 points)
    cout << "Test 1: Create and Load KVS from CSV (25 points)" << endl;
    cout << "----------------------------------------" << endl;

    int create_score = 0;
    rocksdb::DB* db = create_kvs(csv_file_path, db_path);

    if (db != nullptr) {
        cout << "[PASS] RocksDB database created successfully" << endl;
        create_score += 10;

        // Verify data was actually loaded by checking a known key
        string test_value;
        Status s = db->Get(ReadOptions(), "2qh1r_display_name", &test_value);
        if (s.ok() && test_value == "auto") {
            cout << "[PASS] CSV data loaded correctly into database" << endl;
            create_score += 10;
        } else if (s.ok()) {
            cout << "[PARTIAL] Data loaded but values may be incorrect" << endl;
            create_score += 5;
        } else {
            cout << "[FAIL] Database created but data not loaded" << endl;
        }

        // Check if multiple entries exist (verify batch write worked)
        string test_value2;
        Status s2 = db->Get(ReadOptions(), "2qh1k_display_name", &test_value2);
        if (s2.ok()) {
            cout << "[PASS] Multiple entries loaded (batch write implemented)" << endl;
            create_score += 5;
        } else {
            cout << "[FAIL] Batch write may not be working correctly" << endl;
        }

    } else {
        cout << "[FAIL] Failed to create RocksDB database" << endl;
        cout << "Cannot proceed with remaining tests." << endl;
        cout << "Score: 0/25" << endl;
        return 1;
    }

    total_score += create_score;
    cout << "Score: " << create_score << "/25" << endl;

    cout << endl << "========================================" << endl << endl;

    // Test 2: MultiGet operation (25 points)
    cout << "Test 2: MultiGet Operation (25 points)" << endl;
    cout << "----------------------------------------" << endl;
    vector<string> multi_get_keys = {"2qh1r_display_name", "2qh1k_display_name", "2qh1a_display_name", "2qh1b_display_name"};
    vector<string> multi_get_results = multi_get(db, multi_get_keys);

    // Expected results
    vector<string> expected_multi_get = {"auto", "productivity", "linux", "microsoft"};

    int multi_get_score = 0;

    // Check if the correct number of results were returned
    if (multi_get_results.size() == multi_get_keys.size()) {
        cout << "[PASS] Correct number of results returned: " << multi_get_results.size() << endl;
        multi_get_score += 10;
    } else {
        cout << "[FAIL] Expected " << multi_get_keys.size() << " results, got " << multi_get_results.size() << endl;
    }

    // Check if the results match expected values
    int correct_values = 0;
    for (size_t i = 0; i < min(multi_get_results.size(), expected_multi_get.size()); i++) {
        cout << "  Key: " << multi_get_keys[i] << " => Value: " << multi_get_results[i];
        if (multi_get_results[i] == expected_multi_get[i]) {
            cout << " [CORRECT]" << endl;
            correct_values++;
        } else {
            cout << " [INCORRECT - Expected: " << expected_multi_get[i] << "]" << endl;
        }
    }

    if (correct_values == expected_multi_get.size()) {
        cout << "[PASS] All values match expected results" << endl;
        multi_get_score += 15;
    } else {
        cout << "[FAIL] Only " << correct_values << "/" << expected_multi_get.size() << " values are correct" << endl;
        multi_get_score += (correct_values * 15) / expected_multi_get.size();
    }

    total_score += multi_get_score;
    cout << "Score: " << multi_get_score << "/25" << endl;

    cout << endl << "========================================" << endl << endl;

    // Test 3: Range Iteration (35 points)
    cout << "Test 3: Range Iteration (35 points)" << endl;
    cout << "----------------------------------------" << endl;
    vector<string> results = iterate_over_range(db, "2qh0x", "2qh1q");

    int iterate_score = 0;
    const size_t expected_range_size = 51;  // Correct expected size

    // Check if results were returned
    if (results.size() > 0) {
        cout << "[PASS] Iterator returned results" << endl;
        iterate_score += 10;
    } else {
        cout << "[FAIL] Iterator returned no results" << endl;
    }

    // Check if the correct number of results were returned
    cout << "Number of results returned: " << results.size() << endl;
    if (results.size() == expected_range_size) {
        cout << "[PASS] Correct number of results in range (expected " << expected_range_size << ")" << endl;
        iterate_score += 12;
    } else if (results.size() >= expected_range_size - 2 && results.size() <= expected_range_size + 2) {
        cout << "[PARTIAL] Close to expected range size (expected " << expected_range_size << ")" << endl;
        iterate_score += 8;
    } else {
        cout << "[FAIL] Unexpected number of results (expected " << expected_range_size << ", got " << results.size() << ")" << endl;
    }

    // Check if only display_name values are returned (no key strings with underscores)
    bool only_display_names = true;
    int values_with_underscores = 0;
    for (const auto& result : results) {
        if (result.find("_display_name") != string::npos || result.find("2qh") == 0) {
            only_display_names = false;
            values_with_underscores++;
        }
    }

    if (only_display_names) {
        cout << "[PASS] Results contain only display_name values (no keys)" << endl;
        iterate_score += 13;
    } else {
        cout << "[FAIL] Results contain " << values_with_underscores << " key strings instead of values" << endl;
    }

    // Display first 5 results as sample
    cout << "Sample results (first 5):" << endl;
    for (size_t i = 0; i < min(results.size(), size_t(5)); i++) {
        cout << "  " << (i+1) << ". " << results[i] << endl;
    }

    total_score += iterate_score;
    cout << "Score: " << iterate_score << "/35" << endl;

    cout << endl << "========================================" << endl << endl;

    // Test 4: Delete Operation (15 points)
    cout << "Test 4: Delete Operation (15 points)" << endl;
    cout << "----------------------------------------" << endl;

    int delete_score = 0;
    const string key_to_delete = "2cneq_id";

    // First, verify the key exists before deletion
    string value_before;
    Status s_check = db->Get(ReadOptions(), key_to_delete, &value_before);
    if (s_check.ok()) {
        cout << "Key '" << key_to_delete << "' exists before deletion" << endl;
        cout << "Value: " << value_before << endl;
        delete_score += 3;
    } else {
        cout << "[WARNING] Key '" << key_to_delete << "' not found before deletion" << endl;
    }

    // Perform deletion
    Status s = delete_key(db, key_to_delete);
    if (s.ok()) {
        cout << "[PASS] Delete operation returned OK status" << endl;
        delete_score += 6;
    } else {
        cout << "[FAIL] Delete operation failed with status: " << s.ToString() << endl;
    }

    // Verify the key was actually deleted
    string value_after;
    Status s_verify = db->Get(ReadOptions(), key_to_delete, &value_after);
    if (s_verify.IsNotFound()) {
        cout << "[PASS] Key successfully deleted (verified via Get)" << endl;
        delete_score += 6;
    } else if (s_verify.ok()) {
        cout << "[FAIL] Key still exists after deletion" << endl;
        cout << "Value found: " << value_after << endl;
    } else {
        cout << "[FAIL] Error verifying deletion: " << s_verify.ToString() << endl;
    }

    total_score += delete_score;
    cout << "Score: " << delete_score << "/15" << endl;

    cout << endl << "========================================" << endl;
    cout << "         FINAL GRADING SUMMARY" << endl;
    cout << "========================================" << endl;
    cout << "Test 1 - Create & Load KVS: 25 points" << endl;
    cout << "Test 2 - MultiGet:           25 points" << endl;
    cout << "Test 3 - Range Iteration:    35 points" << endl;
    cout << "Test 4 - Delete Operation:   15 points" << endl;
    cout << "----------------------------------------" << endl;
    cout << "TOTAL SCORE: " << total_score << "/" << max_score << endl;

    // Grade letter
    cout << "\nGrade: ";
    if (total_score >= 90) {
        cout << "A (Excellent)" << endl;
    } else if (total_score >= 80) {
        cout << "B (Good)" << endl;
    } else if (total_score >= 70) {
        cout << "C (Satisfactory)" << endl;
    } else if (total_score >= 60) {
        cout << "D (Needs Improvement)" << endl;
    } else {
        cout << "F (Failing)" << endl;
    }
    cout << "========================================" << endl;

    // Clean up
    delete db;

    return 0;
}
