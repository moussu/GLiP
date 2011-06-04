import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import simulator._

package simulator.view {

  object Led {
    val diameter = 10
  }

  class Led(r: Int, g: Int, b: Int) {
    type Color = List[Int]

    private var _color:      Color = List(r, g, b)
    private var _normalized: Color = _color

    def this() = {
      this(0, 0, 0)
      this.randomColor
    }

    def randomColor() = {
      val _rand: Random = new Random()
      _color = List(_rand.nextInt(255), _rand.nextInt(255), _rand.nextInt(255))
    }

    def setColor(r: Int, g: Int, b: Int) = {
      _color = List(r, g, b)
    }

    def color: Color = _color
    def color_=(c: Color) = _color = c

    def draw(context: PApplet) = {
      val a = _color.max.toDouble / 255.0
      val c = _color map (x => (x.toDouble * a + 255.0 * (1 - a)).toInt)
      context.fill(c(0), c(1), c(2))
      context.ellipse(0, 0, Led.diameter, Led.diameter)
    }
  }

}
