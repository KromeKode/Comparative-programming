pub trait Sensor {
    fn read_celsius(&self) -> i32;
}

pub trait Heater {
    fn turn_on(&mut self);
    fn turn_off(&mut self);
}

/// Turns the heater on if the room is more than a degree below `target`, and off if
/// it is more than a degree above. Within a degree, it leaves the heater alone.
pub fn regulate(sensor: &impl Sensor, heater: &mut impl Heater, target: i32) {
    let celsius = sensor.read_celsius();
    if celsius < target - 1 {
        heater.turn_on();
    } else if celsius > target + 1 {
        heater.turn_off();
    }
}

#[cfg(test)]
mod tests;
