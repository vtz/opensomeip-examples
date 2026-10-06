//! Shared helpers for the Rust hello_world example.

use std::thread;
use std::time::{Duration, Instant};

use opensomeip::{Endpoint, SomeIpError, SomeIpMessage, UdpTransport};

pub const SERVICE_ID: u16 = 0x1000;
pub const METHOD_ID: u16 = 0x0001;
pub const SERVER_PORT: u16 = 30490;

/// Poll until a datagram arrives. The C API receive path does not block.
pub fn receive_until(
    transport: &mut UdpTransport,
    timeout: Duration,
) -> Result<(SomeIpMessage, Endpoint), SomeIpError> {
    let start = Instant::now();
    loop {
        match transport.receive() {
            Ok(message) => return Ok(message),
            Err(SomeIpError::Timeout) if start.elapsed() < timeout => {
                thread::sleep(Duration::from_millis(20));
            }
            Err(err) => return Err(err),
        }
    }
}
