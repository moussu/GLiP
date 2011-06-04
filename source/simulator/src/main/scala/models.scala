import processing.core._
import spde.core._
import PConstants._
import scala.math._
import scala.actors._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import java.io._
import java.net._

import simulator.view._

package simulator.ether {
  import Block._

  object Model {
    type IR      = (Block, Dir)
    type Com     = (IR, IR)
    type ComList = List[Com]
    type RoutingTable = HashMap[Dir, Block]
    type Connections = HashMap[Block, RoutingTable]
  }

  abstract class Model {
    import Model._

    protected var connections: Connections = new Connections()
    protected var pool: LinkedList[Block] = LinkedList()
    protected var drawList: ComList = List()
    protected var dirty = true

    def declareDirty() {
      dirty = true
    }

    def resetConnections() {
      drawList = List()
      connections = new Connections()
      for (b <- pool)
        connections(b) = new RoutingTable()
    }

    def send(pool: LinkedList[Block], b: Block, ir: Dir): Option[Block] = {
      connections(b).find(pair => pair._1 == b) foreach {
        case (k, dest) => if (k == ir) return Some(dest)
        case _ => {}
      }

      return None
    }

    def computeConnections(pool: LinkedList[Block])
    def communicates(b1: Block, b2: Block): ComList
    def drawIRs(context: PApplet)
    def drawConnections(context: PApplet)
  }

  class SimpleModel extends Model {
    import Model._

    val maxDist = min(Block.height, Block.width) / 2
    val maxBlockDist = maxDist * 4

    def communicates(b1: Block, b2: Block): ComList = {
      var res: ComList = List()
      for ((d1, p1) <- b1.getIr(); (d2, p2) <- b2.getIr())
        if (PApplet.dist(p1._1, p1._2, p2._1, p2._2) < maxDist)
          res = res :+ ((b1, d1), (b2, d2))
      res
    }

    def computeConnections(pool: LinkedList[Block]) = {
      if (dirty) {
        this.pool = pool

        resetConnections()

        for (i <- 0 until pool.size; j <- (i + 1) until pool.size) {
          val b1 = pool(i)
          val b2 = pool(j)

          if (PApplet.dist(b1.x, b1.y, b2.x, b2.y) < maxBlockDist) {
            val comms = communicates(b1, b2)
            drawList = drawList ++ comms
            for (((b1, d1), (b2, d2)) <- comms) {
              connections(b1)(d1) = b2
              connections(b2)(d2) = b1
            }
          }
        }

        dirty = false
      }
    }

    def drawIRs(context: PApplet) = {
      val d = maxDist

      for (block <- pool) {
        val x = block.x
        val y = block.y

        context.pushMatrix()
        context.translate(x, y)
        context.fill(255, 0, 0, 30)
        context.noStroke()
        context.ellipse(block.stopX  - x, 0, d, d)
        context.ellipse(0, block.stopY  - y, d, d)
        context.ellipse(block.startX - x, 0, d, d)
        context.ellipse(0, block.startY - y, d, d)
        context.popMatrix()
      }
    }

    def drawConnections(context: PApplet) = {
      context.stroke(255, 0, 0, 50)
      context.strokeWeight(3)

      for (c <- drawList) {
        val ((b1, d1), (b2, d2)) = c
        val p1 = b1.getIrAt(d1)
        val p2 = b2.getIrAt(d2)

        context.line(p1._1, p1._2, p2._1, p2._2)
      }

      context.strokeWeight(1)
    }
  }
}
