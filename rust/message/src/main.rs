//! Build a SOME/IP message, serialize it, and read it back.

use opensomeip::{MessageType, ReturnCode, SomeIpError, SomeIpMessage, SomeIpMessageBuilder};

fn main() {
    if let Err(err) = run() {
        eprintln!("Message example failed: {err:?}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), SomeIpError> {
    println!("=== SOME/IP Message Round-Trip (Rust) ===");

    let payload = b"ping";
    let original = SomeIpMessageBuilder::new()?
        .service_id(0x1000)?
        .method_id(0x0001)?
        .client_id(0x1234)?
        .session_id(0x0007)?
        .message_type(MessageType::Request)?
        .return_code(ReturnCode::Ok)?
        .payload(payload)?
        .build();

    let bytes = original.serialize()?;
    println!("Serialized {} bytes", bytes.len());
    if bytes.len() < 16 {
        eprintln!("Serialized message is shorter than a SOME/IP header");
        return Err(SomeIpError::InvalidMessage);
    }

    let mut received = SomeIpMessage::new()?;
    received.deserialize(&bytes)?;

    if received.service_id()? != 0x1000
        || received.method_id()? != 0x0001
        || received.client_id()? != 0x1234
        || received.session_id()? != 0x0007
        || received.message_type()? != MessageType::Request
        || received.payload()? != payload
    {
        eprintln!("Deserialized message does not match");
        return Err(SomeIpError::InvalidMessage);
    }

    println!(
        "Service 0x{:04X} method 0x{:04X}",
        received.service_id()?,
        received.method_id()?
    );
    println!("Payload: {}", String::from_utf8_lossy(&received.payload()?));
    println!("=== Message Round-Trip Complete ===");
    Ok(())
}
