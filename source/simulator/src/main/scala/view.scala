import processing.core._
import PConstants._
import scala.concurrent._
import simulator.ether._

package simulator.view {

  class View(model: Model, val lock: Lock) extends PApplet {
    private val eureka = loadFont("Eureka-90.vlw")
    private val w = 800
    private val h = 600

    private var pool: Pool = new Pool(model, w, h)
    private var selected: Option[Block] = None
    private var detailed: Boolean = true
    private var offsetX:  Int = 0
    private var offsetY:  Int = 0

    def getPool(): Pool = pool

    override def setup() = {
      size(w, h)
      frameRate(30)
      smooth()
      textFont(eureka)
    }

    override def keyPressed() {
      if (key == ' ')
        detailed = ! detailed
    }

    override def mousePressed() {
      if (mouseButton == LEFT) {
        selected = pool.getBlockAt(mouseX, mouseY)
        selected foreach { b => {
            offsetX = mouseX - b.x
            offsetY = mouseY - b.y
          }
        }
      }
      else {
        pool.getBlockAt(mouseX, mouseY) foreach {
          b => pool.rotateBlock(b)
        }
        redraw()
      }
    }

    override def mouseDragged() {
      selected foreach { b => {
        pool.moveBlock(b, mouseX - offsetX, mouseY - offsetY)
        redraw()
      }}
    }

    override def mouseReleased() {
      selected = None
    }

    override def draw() = {
      background(50)
      pool.draw(this, lock, detailed)
    }

    def createFrame() = {
      val frame = new javax.swing.JFrame("Simulator")
      frame.getContentPane().add(this)
      init()
      frame.pack()
      frame.setVisible(true)
    }
  }

}
