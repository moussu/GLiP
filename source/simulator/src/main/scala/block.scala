import processing.core._
import PConstants._
import java.awt.Rectangle

package simulator.view {

  object Block {
    object Dir extends Enumeration {
      type Dir = Value

      val N, S, E, W = Value

      def fromChar(c: Char): Value = {
        c match {
          case 'N' => Dir.N
          case 'W' => Dir.W
          case 'S' => Dir.S
          case 'E' => Dir.E
        }
      }

      def fromInt(n: Int): Value = {
        n match {
          case 0 => Dir.N
          case 1 => Dir.W
          case 2 => Dir.S
          case 3 => Dir.E
        }
      }

      def toInt(d: Value): Int = {
        d match {
          case Dir.N => 0
          case Dir.W => 1
          case Dir.S => 2
          case Dir.E => 3
        }
      }
    }

    type Dir     = Dir.Value
    type Point   = List[Int]

    var count    = 0
    val layout   = List(8, 8)
    val elts     = layout(0) * layout(1)
    val margin   = 2

    def width():  Int = layout(0) * (2 * margin + Led.diameter)
    def height(): Int = layout(1) * (2 * margin + Led.diameter)
    def relativeStartX(): Int = - width  / 2
    def relativeStartY(): Int = - height / 2
    def relativeStopX():  Int =   width  / 2 + 1
    def relativeStopY():  Int =   height / 2 + 1
  }

  class Block(var x: Int, var y: Int) {
    import Block._

    private var angle = 0
    var leds: Array[Array[Led]] = Array.ofDim(layout(0), layout(1))
    val id = count + 1
    count = id

    for (i <- 0 until layout(1); j <- 0 until layout(0)) {
      leds(i)(j) = new Led()
    }

    def this() = this(0, 0)

    def startX(): Int = x + relativeStartX
    def startY(): Int = y + relativeStartY
    def stopX():  Int = x + relativeStopX
    def stopY():  Int = y + relativeStopY
    def addr() = id * 4
    def addr(iface: Dir) = id * 4 + Dir.toInt(iface)

    override def toString() = {
      "[" + id.toString + "]"
    }

    def move(newX: Int, newY: Int) = {
      x = newX
      y = newY
    }

    def ledAt(x: Int, y: Int): Led = {
      leds(y)(x)
    }

    def isInside(x: Int, y: Int): Boolean = {
      if (x < startX || x >= startX + width)
        return false
      if (y < startY || y >= startY + height)
        return false
      return true
    }

    def asRectangle(): `Rectangle` =
      new `Rectangle`(x, y, width, height)

    def intersects(other: Block): Boolean = {
      asRectangle().intersects(other.asRectangle)
    }

    def getIrAt(dir: Dir): (Int, Int) = {
      getIr()(dir)
    }

    def getIr() = {
      val dirs = Array(Dir.N, Dir.W, Dir.S, Dir.E)
      val points = Array((x,        startY()),
                         (startX(), y),
                         (x,        stopY()),
                         (stopX(),  y))

      var map = Map((dirs(angle / 90 % 4), points(0)))
      for (i <- 1 to 3) {
        map += ((dirs((i + angle / 90) % 4), points(i)))
      }
      map
    }

    def rotate() = {
      angle += 90
      angle = angle % 360
    }

    def draw(context: PApplet, detailed: Boolean) = {
      context.pushMatrix()
      context.translate(x, y)
      context.rotate(angle * PI / 180.0f)
      context.fill(0)
      context.stroke(255)
      context.rect(startX - x, startY - y, width, height)

      for (i <- 0 until layout(1); j <- 0 until layout(0)) {
        val offset =  + Led.diameter / 2 + margin
        val x: Int = (j - layout(0) / 2) *
                     (2 * margin + Led.diameter) + offset
        val y: Int = (i - layout(1) / 2) *
                     (2 * margin + Led.diameter) + offset
        context.pushMatrix()
        context.fill(20)
        context
        context.translate(x, y);
        leds(i)(j).draw(context)
        context.popMatrix()
      }

      if (detailed) {
        context.fill(255, 255, 255, 200)

        context.textSize(20)
        context.pushMatrix()
        context.rotate(-angle * PI / 180.f)
        context.scale(0.7f, 0.8f)
        for ((d, p) <- getIr()) {
          val text = d.toString + "(" + addr(d) + ")"
          val offsetX = context.textWidth(text) / 2
          val offsetY = 10
          context.text(text, p._1 - x - offsetX,
                             p._2 - y - offsetY, 100, 100)
        }
        context.popMatrix()

        val text = if (id < 10) "0" + id else id.toString
        val address = "0x" + addr().toString
        context.textSize(45)
        context.text(text, -context.textWidth(text) / 2,
                     -30, width, height)
        context.textSize(25)
        context.text(address, -context.textWidth(address) / 2,
                     5, width, height)
      }

      context.popMatrix()
    }
  }

}
