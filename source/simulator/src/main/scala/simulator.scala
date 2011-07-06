import scala.concurrent._
import simulator.ether._
import simulator.view._

package simulator {

  object Simulator {
    private val lock = new Lock

    def main(args: Array[String]) {
      var bufferSize = 65535

      if (args.size == 1)
        bufferSize = args(0).toInt

      val view   = new   View(SimpleModel,  lock)
      val server = new Server(view.getPool, lock, bufferSize)

      view.createFrame()
      server.start()
    }
  }

}
