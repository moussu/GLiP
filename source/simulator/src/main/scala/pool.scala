import processing.core._
import scala.collection.mutable._
import scala.concurrent._
import simulator.ether._
import java.awt.Rectangle
import scala.math._
import compat.Platform._

package simulator.view {

  class Pool(val model: Model,
             val windowWidth: Int, val windowHeight: Int,
             var randomAngle: Boolean) {
    import Model._
    import Block._

    private var pool: LinkedList[Block] = LinkedList()
    val padding: Int = 10
    val minPadding: Int = 2
    val maxHeight: Int = 4 * (Block.height + padding)
    val maxWidth:  Int = 4 * (Block.width  + padding)

    /*
     * Padding strategy:
     *
     *     3 3 3 3
     *     2 2 2 3
     *     1 1 2 3
     *     0 1 2 3
     *
     *     l = 0 => (0, 0); k = 0
     *     l = 1 => (0, 1), (1, 1), (1, 0); k = 0 .. 2
     *     l = 2 => (0, 2), (1, 2), (2, 2), (2, 1), (2, 0); k = 0 .. 4
     *     ...
     *
     *     n_l = 2 * (l + 1) - 1
     *     k_l = 0 .. n_l - 1
     */

    var k = 0
    var l = 0
    var n = 1

    def offsetX(w: Int): Int = (Block.width  + windowWidth  - w) / 2 + padding
    def offsetY(h: Int): Int = (Block.height + windowHeight - h) / 2 + padding

    def addBlock(): Block = {
      val block: Block = new Block(randomAngle)
      val (i, j) = if (k <= l) (k, l) else (l, 2 * l - k)
      val x = j * (Block.width  + padding) + offsetX(maxWidth)
      val y = i * (Block.height + padding) + offsetY(maxHeight)

      k = k + 1
      if (k == n) {
        k = 0
        l = l + 1
        n = 2 * (l + 1) - 1
      }

      block.move(x, y)
      pool = pool :+ block
      model.declareDirty()

      block
    }

    def generateBlockMatrix(n: Int) = {
      val width:  Int = n * (Block.width  + padding)
      val height: Int = n * (Block.height + padding)

      val offsetX = this.offsetX(width)
      val offsetY = this.offsetY(height)

      for (i <- 0 until n; j <- 0 until n) {
        val block: Block = new Block(randomAngle)

        val x: Int = j * (Block.width  + padding) + offsetX
        val y: Int = i * (Block.height + padding) + offsetY

        block.move(x, y)
        pool = pool :+ block
      }

      model.declareDirty()
    }

    def :+ (b: Block) = {
      if (isRoomFor(b)) {
        pool = pool :+ b
        model.declareDirty()
      }
    }

    def putBlockAt(b: Block, x: Int, y: Int) = {
      if (isRoomFor(x, y, Block.width, Block.height)) {
        b.move(x, y)
        pool.insert(LinkedList(b))
        model.declareDirty()
      }
    }

    def getBlockAt(x: Int, y: Int): Option[Block] = {
      for (b <- pool) {
        if (b.isInside(x, y))
          // One has to declare the model dirty depending on what will
          // be done with that block.
          return Some(b)
      }

      println("No block")
      return None
    }

    def moveBlockTowards(b: Block, x: Int, y: Int, speed: Int, context: PApplet) = {
      val dx = x - b.x
      val dy = y - b.y
      val norm = sqrt(dx * dx + dy * dy)
      val ux = speed * dx / norm
      val uy = speed * dy / norm

      if (norm < speed)
        moveBlock(b, x, y, context)
      else if ((ux, uy) != (0, 0))
        moveBlock(b, b.x + ux.toInt, b.y + uy.toInt, context)
    }

    def moveBlock(b: Block, x: Int, y: Int, context: PApplet): Unit = {
      var p = capValues(b, x, y, context)
      if ((b.x, b.y) != (p._1, p._2)) {
        b.move(p._1, p._2)
        model.declareDirty
      }
    }

    def rotateBlock(b: Block) = {
      b.rotate()
      model.declareDirty()
    }

    def isRoomFor(b: Block): Boolean = {
      return pool.filter(x => x != b && x.intersects(b)).length == 0
    }

    def capValues(block: Block, x: Int, y: Int, context: PApplet): (Int, Int) = {
      val newB = new Block(x, y)
      val blockr = block.asRectangle

      for (b <- pool) {
        if (b != block) {
          val br = b.asRectangle
          var draw = false

          if (b.intersects(newB, minPadding / 2)) {
            draw = true
            if (abs(b.x - newB.x) < Block.width + minPadding &&
                abs(b.x - block.x) > abs(b.y - block.y))
              newB.x = b.x + signum(newB.x - b.x) * (Block.width + minPadding)
            else if (abs(b.y - newB.y) < Block.height + minPadding &&
                     abs(b.y - block.y) > abs(b.x - block.x))
              newB.y = b.y + signum(newB.y - b.y) * (Block.height + minPadding)
          }

          for ((dx, dy) <- List(( Block.width / 2,  Block.height / 2),
                                (-Block.width / 2,  Block.height / 2),
                                ( Block.width / 2, -Block.height / 2),
                                (-Block.width / 2, -Block.height / 2))) {
            if (br.intersectsLine(block.x + dx, block.y + dy, x + dx, y + dy)) {
              draw = true
              if (abs(b.x - block.x) > abs(b.y - block.y))
                newB.x = b.x + signum(block.x - b.x) * (Block.width + minPadding)
              else if (abs(b.y - block.y) > abs(b.x - block.x))
                newB.y = b.y + signum(block.y - b.y) * (Block.height + minPadding)
            }
          }

          if (draw) {
            b.mark(255, 0, 0)
            block.mark(255, 0, 0)
          }
        }
      }

      return (newB.x, newB.y)
    }

    def isRoomFor(x: Int, y: Int, width: Int, height: Int): Boolean = {
      val r = new `Rectangle`(x - width / 2, y - height / 2, width, height)
      return pool.filter(b => b.asRectangle.intersects(r)).length == 0
    }

    def send(b: Block, ir: Dir) = {
      model.send(pool, b, ir)
    }

    def refresh() = {
      model.computeConnections(pool)
    }

    def draw(context: PApplet, lock: Lock, detailed: Boolean) = {
      lock.acquire
      refresh
      lock.release

      if (detailed)
        model.drawIRs(context)

      model.drawConnections(context)

      pool foreach { b => b.draw(context, detailed) }
    }
  }





}
