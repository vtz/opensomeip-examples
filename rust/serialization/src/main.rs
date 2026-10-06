//! Integer serialization round-trip using the opensomeip Rust bindings.

use opensomeip::{Deserializer, Serializer, SomeIpError};

fn main() {
    if let Err(err) = run() {
        eprintln!("Serialization example failed: {err:?}");
        std::process::exit(1);
    }
}

fn run() -> Result<(), SomeIpError> {
    println!("=== SOME/IP Serialization (Rust) ===");

    let mut serializer = Serializer::new()?;
    serializer.write_u8(0x42)?;
    serializer.write_u16(0x1234)?;
    serializer.write_u32(0xDEAD_BEEF)?;
    serializer.write_u64(0x0123_4567_89AB_CDEF)?;
    let encoded = serializer.to_vec()?;
    println!("Serialized {} bytes", encoded.len());

    let mut deserializer = Deserializer::new(&encoded)?;
    let u8_value = deserializer.read_u8()?;
    let u16_value = deserializer.read_u16()?;
    let u32_value = deserializer.read_u32()?;
    let u64_value = deserializer.read_u64()?;
    if deserializer.remaining()? != 0 {
        eprintln!("Unexpected trailing bytes");
        return Err(SomeIpError::MalformedMessage);
    }

    println!("u8  = 0x{u8_value:02X}");
    println!("u16 = 0x{u16_value:04X}");
    println!("u32 = 0x{u32_value:08X}");
    println!("u64 = 0x{u64_value:016X}");

    if u8_value != 0x42
        || u16_value != 0x1234
        || u32_value != 0xDEAD_BEEF
        || u64_value != 0x0123_4567_89AB_CDEF
    {
        eprintln!("Decoded values do not match");
        return Err(SomeIpError::MalformedMessage);
    }

    println!("=== Serialization Demo Complete ===");
    Ok(())
}
