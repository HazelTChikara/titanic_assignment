# NoSQL Assignment - RocksDB Setup

This assignment requires RocksDB 7.10.2 to be installed and built from source.

## Prerequisites

### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required dependencies
brew install lz4 zstd bzip2 snappy gflags
```

### Linux
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential libgflags-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev

# RHEL/CentOS
sudo yum install -y gcc-c++ gflags-devel snappy-devel zlib-devel bzip2-devel lz4-devel libzstd-devel
```

## RocksDB 7.10.2 Installation

### Download and Build
```bash
# Download RocksDB 7.10.2
curl -L https://github.com/facebook/rocksdb/archive/refs/tags/v7.10.2.tar.gz -o rocksdb-7.10.2.tar.gz

# Extract
tar -xzf rocksdb-7.10.2.tar.gz

# Move to home directory (to avoid path issues with spaces)
mv rocksdb-7.10.2 ~/rocksdb-7.10.2

# Build static library
cd ~/rocksdb-7.10.2
EXTRA_CXXFLAGS="-Wno-error=unused-but-set-variable" make static_lib -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
```

This will create `librocksdb.a` in the `~/rocksdb-7.10.2` directory.

## Data Setup

You need to download the `subreddits.csv` file from the assignment instructions and place it in this directory.

## Build and Test

```bash
# Create test directory
mkdir -p test

# Build the project
make

# Run tests
./tester
```

## Makefile Configuration

The Makefile is pre-configured to use:
- **RocksDB**: `~/rocksdb-7.10.2`
- **Homebrew libraries**: `/opt/homebrew/lib` (macOS ARM)

If you're on Linux or x86 Mac, update the `ROCKSDB_DIR` in the Makefile accordingly.

## Expected Output

When tests run successfully, you should see:
```
TOTAL SCORE: 100/100
Grade: A (Excellent)
```

## Submission

Submit only the `assignment-nosql.cc` file to Canvas.

## Troubleshooting

### Issue: Cannot find RocksDB headers
**Solution**: Update `ROCKSDB_DIR` in Makefile to point to your RocksDB installation

### Issue: lz4 library not found
**Solution**: 
- macOS: `brew install lz4`
- Linux: Install via package manager as shown above

### Issue: Compilation errors with unused variables
**Solution**: Use `EXTRA_CXXFLAGS="-Wno-error=unused-but-set-variable"` when building RocksDB
