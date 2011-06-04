import processing.core._
import spde.core._
import PConstants._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle

package simulator {
  import ether._
  import view._

  object Simulator {
    def main(args: Array[String]) {
      View.start()
      new Server(View.pool).start()
    }
  }

}
