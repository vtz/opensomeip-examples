//! Hello World client -- sends a greeting and checks the server response.

use std::time::Duration;

use hello_world::{receive_until, METHOD_ID, SERVER_PORT, SERVICE_ID};
use opensomeip::{
    Endpoint, MessageType, ReturnCode, SomeIpError, SomeIpMessageBuilder, TransportProtocol,
    UdpTransport,
};

fn main() {
    if let Err(err) = run() {
        eprintln!("Hello World client failed: {err:?}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), SomeIpError> {
    let local = Endpoint::new("127.0.0.1", 0, TransportProtocol::Udp);
    let mut transport = UdpTransport::new(&local)?;
    transport.start()?;

    let greeting = b"Hello from Rust Client!";
    let request = SomeIpMessageBuilder::new()?
        .service_id(SERVICE_ID)?
        .method_id(METHOD_ID)?
        .client_id(0x1234)?
        .session_id(0x0001)?
        .message_type(MessageType::Request)?
        .return_code(ReturnCode::Ok)?
        .payload(greeting)?
        .build();

    let server = Endpoint::new("127.0.0.1", SERVER_PORT, TransportProtocol::Udp);
    println!("=== SOME/IP Hello World Client (Rust) ===");
    println!("Sending message to 127.0.0.1:{SERVER_PORT}");
    transport.send(&request, &server)?;

    let (response, _) = receive_until(&mut transport, Duration::from_secs(5))?;
    if response.message_type()? != MessageType::Response {
        eprintln!("Unexpected message type");
        return Err(SomeIpError::InvalidMessageType);
    }
    let text = String::from_utf8_lossy(&response.payload()?).into_owned();
    println!("Server responded: '{text}'");
    if !text.contains("Hello World!") {
        eprintln!("Response did not contain the expected greeting");
        return Err(SomeIpError::InvalidMessage);
    }

    transport.stop()?;
    println!("Client finished.");
    Ok(())
}
