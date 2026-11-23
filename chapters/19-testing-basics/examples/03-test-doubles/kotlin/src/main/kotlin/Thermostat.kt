// A fun interface has one function, so a lambda can stand for it.
fun interface Sensor {
    fun readCelsius(): Int
}

interface Heater {
    fun turnOn()

    fun turnOff()
}

/**
 * Turns the heater on if the room is more than a degree below [target], and off if it is more than
 * a degree above. Within a degree, it leaves the heater alone.
 */
fun regulate(sensor: Sensor, heater: Heater, target: Int) {
    val celsius = sensor.readCelsius()
    if (celsius < target - 1) {
        heater.turnOn()
    } else if (celsius > target + 1) {
        heater.turnOff()
    }
}
