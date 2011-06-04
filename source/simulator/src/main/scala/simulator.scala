import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import scala.concurrent._

package simulator {
  import ether._
  import view._

  object Simulator {
    private val lock = new Lock

    def main(args: Array[String]) {
      val view   = new   View(SimpleModel, lock)
      val server = new Server(view.getPool, lock)

      view.createFrame()
      server.start()
    }
  }

}
