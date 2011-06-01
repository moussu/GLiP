import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle

object StraightScalaRunner {
  def main(args: Array[String]) { PApplet.main(Array("Simulator")) }
}

class Simulator extends PApplet {
  var pool: Pool = new Pool
  var selected: Option[Block] = None
  var offsetX: Int = 0
  var offsetY: Int = 0

  override def setup() = {
    size(800, 600)
    frameRate(30)
    smooth()

    pool.generateBlockMatrix(4, width, height)
  }

  override def mousePressed() {
    selected = pool.getBlockAt(mouseX, mouseY)
    selected match {
      case Some(b) => {
        offsetX = mouseX - b.x
        offsetY = mouseY - b.y
      }
      case None =>
    }
  }

  override def mouseDragged() {
    if (selected != None && pool.getBlockAt(mouseX, mouseY) != selected)
      selected = None
    else
      selected match {
        case Some(b) => {
          pool.moveBlock(b, mouseX - offsetX, mouseY - offsetY)
          redraw()
        }
        case None =>
      }
  }

  override def mouseReleased() {
    selected = None
  }

  override def draw() = {
    background(50)
    pool.draw(this)
  }
}

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

  def color: Color = _color
  def color_=(c: Color) = _color = c

  def draw(context: PApplet) = {
    var a: Double = _color.max.toDouble / 255.0
    var c: Color = _color map (x => (x.toDouble * a + 255.0 * (1 - a)).toInt)
    context.fill(c(0), c(1), c(2))
    context.ellipse(0, 0, Led.diameter, Led.diameter)
  }
}

class Block(var x: Int, var y: Int) {
  type Point = List[Int]

  val margin = 2

  var selected = false
  private val _layout: Point = List(8, 8)
  private val _elts:   Int   = _layout(0) * _layout(1)
  private var _leds:   Array[Array[Led]] =
    Array.ofDim(_layout(0), _layout(1))

  for (i <- 0 until _layout(1); j <- 0 until _layout(0)) {
    _leds(i)(j) = new Led
  }

  def this() = this(0, 0)

  def move(newX: Int, newY: Int) = {
    x = newX
    y = newY
  }

  def ledAt(x: Int, y: Int): Led = {
    _leds(y)(x)
  }

  def width(): Int = (_layout(0) + 1) * (2 * margin) + _layout(0) * Led.diameter
  def height(): Int = (_layout(1) + 1) * (2 * margin) + _layout(1) * Led.diameter
  def startx(): Int = x -(width + Led.diameter) / 2 - margin
  def starty(): Int = y -(height + Led.diameter) / 2 - margin
  def stopx(): Int = x + (width + Led.diameter) / 2 + margin
  def stopy(): Int = y + (height + Led.diameter) / 2 + margin

  def isInside(x: Int, y: Int): Boolean = {
    if (x < startx || x >= startx + width)
      return false
    if (y < starty || y >= starty + height)
      return false
    return true
  }

  def asRectangle(): `Rectangle` = new `Rectangle`(x, y, width, height)

  def intersects(other: Block): Boolean = {
    return asRectangle().intersects(other.asRectangle)
  }

  def leds: Array[Array[Led]] = _leds

  def draw(context: PApplet) = {
    if (selected)
      context.fill(125, 0, 0, 0)

    context.pushMatrix()
    context.translate(x, y)
    context.fill(0)
    context.stroke(255)
    context.rect(startx - x, starty - y, width, height)

    context.noStroke()
    for (i <- 0 until _layout(1); j <- 0 until _layout(0)) {
      val x: Int = (j - _layout(0) / 2) * (2 * margin + Led.diameter)
      val y: Int = (i - _layout(1) / 2) * (2 * margin + Led.diameter)
      context.pushMatrix()
      context.fill(20)
      context
      context.translate(x, y);
      _leds(i)(j).draw(context)
      context.popMatrix()
    }
    context.popMatrix()
  }
}

class Pool {
  var pool: LinkedList[Block] = LinkedList()

  def :+ (b: Block) = {
    if (isRoomFor(b))
      pool = pool :+ b
      // Isn't there any way to do pool.append(b)
      // without any new List ???
  }

  def putBlockAt (newB: Block, x: Int, y: Int) = {
    val oldX: Int = newB.x
    val oldY: Int = newB.y

    newB.move(x, y)

    if (isRoomFor(newB))
      pool = pool :+ newB
    else
      newB.move(oldX, oldY)
  }

  def getBlockAt(x: Int, y: Int): Option[Block] = {
    for (b <- pool) {
      if (b.isInside(x, y))
        return Some(b)
    }
    println("No block")
    return None
  }

  def moveBlock(b: Block, x: Int, y: Int) = {
    val oldX: Int = b.x
    val oldY: Int = b.y

    b.move(x, y)

    if (! isRoomFor(b))
      b.move(oldX, oldY)
  }

  def isRoomFor(b: Block) = {
    pool.filter(x => x != b && x.intersects(b)).length == 0
  }

  def generateBlockMatrix(n: Int, windowWidth: Int, windowHeight: Int) = {
    val padding: Int = 10

    for (i <- 0 until n; j <- 0 until n) {
      val block: Block = new Block()
      val height: Int = n * (block.height + padding)
      val width:  Int = n * (block.width  + padding)
      val offsetX: Int = (block.width  + windowWidth  - width)  / 2 + padding
      val offsetY: Int = (block.height + windowHeight - height) / 2 + padding
      val x: Int = j * (block.width  + padding) + offsetX
      val y: Int = i * (block.height + padding) + offsetY
      block.move(x, y)
      pool = pool :+ block
    }
  }

  private var cnt: Int = 0
  def draw(context: PApplet) = {
    for (b <- pool) {
      if (cnt == 0)
        b.leds.foreach(row => row.foreach(led => led.randomColor()))
      b.draw(context)
    }

    cnt = cnt + 1
    if (cnt == 10)
    {
      cnt = 0
    }
  }
}
