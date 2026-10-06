//! Hello World server -- one SOME/IP request/response over UDP.

use std::time::Duration;

use hello_world::{receive_until, METHOD_ID, SERVER_PORT, SERVICE_ID};
use opensomeip::{
    Endpoint, MessageType, ReturnCode, SomeIpError, SomeIpMessageBuilder, TransportProtocol,
    UdpTransport,
};

fn main() {
    if let Err(err) = run() {
        eprintln!("Hello World server failed: {err:?}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), SomeIpError> {
    let local = Endpoint::new("127.0.0.1", SERVER_PORT, TransportProtocol::Udp);
    let mut transport = UdpTransport::new(&local)?;
    transport.start()?;

    println!("=== SOME/IP Hello World Server (Rust) ===");
    println!("Listening on 127.0.0.1:{SERVER_PORT}");

    let (request, sender) = receive_until(&mut transport, Duration::from_secs(10))?;
    if request.service_id()? != SERVICE_ID || request.method_id()? != METHOD_ID {
        eprintln!("Unexpected request");
        return Err(SomeIpError::InvalidMessage);
    }

    let text = String::from_utf8_lossy(&request.payload()?).into_owned();
    println!(
        "Client said: '{text}' (from {}:{})",
        sender.address_str(),
        sender.port
    );

    let greeting = format!("Hello World! Server received: {text}");
    let response = SomeIpMessageBuilder::new()?
        .service_id(SERVICE_ID)?
        .method_id(METHOD_ID)?
        .client_id(request.client_id()?)?
        .session_id(request.session_id()?)?
        .message_type(MessageType::Response)?
        .return_code(ReturnCode::Ok)?
        .payload(greeting.as_bytes())?
        .build();
    transport.send(&response, &sender)?;
    println!("Sent greeting: '{greeting}'");

    transport.stop()?;
    println!("Server stopped.");
    Ok(())
}
