import kotlin.test.Test
import kotlin.test.assertEquals

/** A spy: a heater that records the calls made to it, for the test to check. */
class SpyHeater : Heater {
    val calls = mutableListOf<String>()

    override fun turnOn() {
        calls += "on"
    }

    override fun turnOff() {
        calls += "off"
    }
}

class ThermostatTest {
    // The stubs are lambdas: Sensor { 17 } is a sensor that always reads 17.
    @Test
    fun tooCold() {
        val heater = SpyHeater()
        regulate(Sensor { 17 }, heater, 20)
        assertEquals(listOf("on"), heater.calls)
    }

    @Test
    fun tooHot() {
        val heater = SpyHeater()
        regulate(Sensor { 23 }, heater, 20)
        assertEquals(listOf("off"), heater.calls)
    }

    @Test
    fun closeEnough() {
        for (celsius in 19..21) {
            val heater = SpyHeater()
            regulate(Sensor { celsius }, heater, 20)
            assertEquals(emptyList(), heater.calls, "at $celsius")
        }
    }
}
