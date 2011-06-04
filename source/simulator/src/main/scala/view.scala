import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import scala.actors._
import simulator.ether._

package simulator.view {

  class View(model: Model) extends PApplet {
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
      for (i <- 0 until 9)
        pool.addBlock()
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
      pool.draw(this, detailed)
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
