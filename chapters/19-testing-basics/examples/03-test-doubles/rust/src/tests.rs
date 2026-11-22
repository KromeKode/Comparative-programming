use super::*;

/// A stub: a sensor that reads whatever temperature the test gives it.
struct StubSensor(i32);

impl Sensor for StubSensor {
    fn read_celsius(&self) -> i32 {
        self.0
    }
}

/// A spy: a heater that records the calls made to it, for the test to check.
#[derive(Default)]
struct SpyHeater {
    calls: Vec<&'static str>,
}

impl Heater for SpyHeater {
    fn turn_on(&mut self) {
        self.calls.push("on");
    }
    fn turn_off(&mut self) {
        self.calls.push("off");
    }
}

#[test]
fn too_cold() {
    let mut heater = SpyHeater::default();
    regulate(&StubSensor(17), &mut heater, 20);
    assert_eq!(heater.calls, ["on"]);
}

#[test]
fn too_hot() {
    let mut heater = SpyHeater::default();
    regulate(&StubSensor(23), &mut heater, 20);
    assert_eq!(heater.calls, ["off"]);
}

#[test]
fn close_enough() {
    for celsius in 19..=21 {
        let mut heater = SpyHeater::default();
        regulate(&StubSensor(celsius), &mut heater, 20);
        assert!(heater.calls.is_empty(), "at {celsius}: {:?}", heater.calls);
    }
}
