# Rust examples

Examples that use the safe Rust bindings in [opensomeip-rs-bind](https://github.com/vtz/opensomeip-rs-bind) (`feature/rust-bindings`). They link the opensomeip C API, so install opensomeip v0.2.0 with `BUILD_CAPI=ON` first.

```bash
cmake -S /path/to/opensomeip -B /tmp/opensomeip-build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/opensomeip-install \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DBUILD_CAPI=ON -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
cmake --build /tmp/opensomeip-build -j
cmake --install /tmp/opensomeip-build

export OPENSOMEIP_DIR=$HOME/opensomeip-install
cd rust
cargo run -p serialization
cargo run -p message

# Terminal 1
cargo run -p hello_world --bin hello_world_server
# Terminal 2
cargo run -p hello_world --bin hello_world_client
```

`hello_world` uses service `0x1000` / method `0x0001` on UDP `127.0.0.1:30490`, the same port as `config/hello_world.yaml`.
