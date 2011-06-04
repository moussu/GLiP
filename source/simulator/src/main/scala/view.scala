import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import scala.actors._
import simulator.ether._

package simulator.view {

  object View extends Actor {
    var pool: Pool = null

    def act() =
      PApplet.main(Array("simulator.view.View"))
  }

  class View extends PApplet {
    import View._

    var selected: Option[Block] = None
    var detailed: Boolean = true
    var offsetX: Int = 0
    var offsetY: Int = 0
    val eureka = loadFont("Eureka-90.vlw")

    override def setup() = {
      size(800, 600)
      frameRate(30)
      smooth()
      textFont(eureka)
      pool = new Pool(width, height)
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
  }

}
