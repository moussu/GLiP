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
  val eureka = loadFont("Eureka-90.vlw")

  override def setup() = {
    size(800, 600)
    frameRate(30)
    smooth()
    textFont(eureka)
    pool.generateBlockMatrix(4, width, height)
  }

  override def keyPressed() {
    if (key == ' ') {
      Block.detailed = ! Block.detailed
    }
  }

  override def mousePressed() {
    if (mouseButton == LEFT) {
      selected = pool.getBlockAt(mouseX, mouseY)
      selected match {
        case Some(b) => {
          offsetX = mouseX - b.x
          offsetY = mouseY - b.y
        }
        case None =>
      }
    }
    else {
      pool.getBlockAt(mouseX, mouseY) foreach { b => b.rotate() }
      redraw()
    }
  }

  override def mouseDragged() {
    if (selected != None && pool.getBlockAt(mouseX, mouseY) != selected)
      selected = None
    else
      selected foreach { b =>
          pool.moveBlock(b, mouseX - offsetX, mouseY - offsetY)
          redraw()
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
    val a = _color.max.toDouble / 255.0
    val c = _color map (x => (x.toDouble * a + 255.0 * (1 - a)).toInt)
    context.fill(c(0), c(1), c(2))
    context.ellipse(0, 0, Led.diameter, Led.diameter)
  }
}

object Block {
  object Dir extends Enumeration {
    type Dir = Value
    val N, S, E, W = Value
  }

  type Dir = Dir.Value
  type IR = (Block, Dir)
  type Com = (IR, IR)
  type ComList = List[Com]
  var count = 0
  var detailed = true

  def drawConnection(context: PApplet, cn: Com) = {
    val ir1 = cn._1
    val b1 = ir1._1
    val d1 = ir1._2

    val ir2 = cn._2
    val b2 = ir2._1
    val d2 = ir2._2

    var p1 = b1.getIrAt(d1)
    var p2 = b2.getIrAt(d2)

    context.line(p1._1, p1._2, p2._1, p2._2)
  }
}

class Block(var x: Int, var y: Int) {
  import Block._

  type Point = List[Int]

  val margin = 2

  private var id = count + 1
  private var angle = 0
  private val _layout: Point = List(8, 8)
  private val _elts:   Int   = _layout(0) * _layout(1)
  private var _leds:   Array[Array[Led]] =
    Array.ofDim(_layout(0), _layout(1))

  count = id
  for (i <- 0 until _layout(1); j <- 0 until _layout(0)) {
    _leds(i)(j) = new Led
  }

  def this() = this(0, 0)

  override def toString() = {
    "[" + id.toString + "]"
  }

  def move(newX: Int, newY: Int) = {
    x = newX
    y = newY
  }

  def ledAt(x: Int, y: Int): Led = {
    _leds(y)(x)
  }

  def width(): Int = _layout(0) * (2 * margin + Led.diameter)
  def height(): Int = _layout(1) * (2 * margin + Led.diameter)
  def startx(): Int = x - width  / 2
  def starty(): Int = y - height / 2
  def stopx(): Int = x + width  / 2 + 1
  def stopy(): Int = y + height / 2 + 1

  def isInside(x: Int, y: Int): Boolean = {
    if (x < startx || x >= startx + width)
      return false
    if (y < starty || y >= starty + height)
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
    val points = Array((x,        starty()),
                       (startx(), y),
                       (x,        stopy()),
                       (stopx(),  y))

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

  def communicates(other: Block): ComList = {
    val maxDist = 50.0
    var res: ComList = List()
    for ((d1, p1) <- getIr(); (d2, p2) <- other.getIr()) {
      if (PApplet.dist(p1._1, p1._2, p2._1, p2._2) < maxDist) {
        res = res :+ ((this, d1), (other, d2))
      }
    }
    res
  }

  def leds: Array[Array[Led]] = _leds

  def drawIR(context: PApplet) = {
    if (detailed) {
      context.pushMatrix()
      context.translate(x, y)
      context.fill(255, 0, 0, 30)
      context.noStroke()
      context.ellipse(stopx - x,  0,          50, 50)
      context.ellipse(0,          stopy - y,  50, 50)
      context.ellipse(startx - x, 0,          50, 50)
      context.ellipse(0,          starty - y, 50, 50)
      context.popMatrix()
    }
  }

  def draw(context: PApplet) = {
    context.pushMatrix()
    context.translate(x, y)
    context.rotate(angle * PI / 180.0f)
    context.fill(0)
    context.stroke(255)
    context.rect(startx - x, starty - y, width, height)

    context.noStroke()
    for (i <- 0 until _layout(1); j <- 0 until _layout(0)) {
      val offset =  + Led.diameter / 2 + margin
      val x: Int = (j - _layout(0) / 2) *
                   (2 * margin + Led.diameter) + offset
      val y: Int = (i - _layout(1) / 2) *
                   (2 * margin + Led.diameter) + offset
      context.pushMatrix()
      context.fill(20)
      context
      context.translate(x, y);
      _leds(i)(j).draw(context)
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
      context.text(text, -context.textWidth(text) / 2, -35, width, height)
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
    pool foreach { b =>
      b.drawIR(context)
    }

    context.stroke(255, 0, 0, 50)
    context.strokeWeight(3)
    for (i <- 0 until pool.size; j <- i + 1 until pool.size) {
      val b1 = pool(i)
      val b2 = pool(j)
      val maxDist = b1.width / 2 + b2.width / 2 + 50.0
      if (PApplet.dist(b1.x, b1.y, b2.x, b2.y) < maxDist) {
        for (cn <- b1 communicates b2) {
          Block.drawConnection(context, cn)
        }
      }
    }
    context.strokeWeight(1)

    pool foreach { b =>
      if (cnt == 0)
        b.leds.foreach(row =>
          row.foreach(led => led.randomColor()))
      b.draw(context)
    }

    cnt = cnt + 1
    if (cnt == 10)
    {
      cnt = 0
    }
  }
}
