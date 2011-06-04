import processing.core._
import PConstants._
import java.awt.Rectangle

package simulator.view {

  object Block {
    object Dir extends Enumeration {
      type Dir = Value
      val N, S, E, W = Value
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

        context.textSize(30)
        context.pushMatrix()
        context.rotate(-angle * PI / 180.f)
        context.scale(0.8f)
        for ((d, p) <- getIr()) {
          val text = d.toString
          val offsetX = context.textWidth(text) / 2
          val offsetY = 12
          context.text(text, p._1 - x - offsetX,
                             p._2 - y - offsetY, 100, 100)
        }
        context.popMatrix()

        val text = if (id < 10) "0" + id else id.toString
        context.textSize(70)
        context.text(text, -context.textWidth(text) / 2,
                     -35, width, height)
      }

      context.popMatrix()
    }
  }

}
