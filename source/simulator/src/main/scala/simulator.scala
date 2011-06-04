import scala.concurrent._
import simulator.ether._
import simulator.view._

package simulator {

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
