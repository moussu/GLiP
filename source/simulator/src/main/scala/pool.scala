import processing.core._
import scala.collection.mutable._
import scala.concurrent._
import simulator.ether._

package simulator.view {

  class Pool(val model: Model, val windowWidth: Int, val windowHeight: Int) {
    import Model._
    import Block._

    private var pool: LinkedList[Block] = LinkedList()
    val padding: Int = 10
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
    def nextIJ(k: Int, l: Int): (Int, Int) =
      if (k <= l) (k, l) else (l, 2 * l - k)

    def addBlock(): Block = {
      val block: Block = new Block()
      val (i, j) = nextIJ(k, l)
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
        val block: Block = new Block()

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

    def putBlockAt (newB: Block, x: Int, y: Int) = {
      val oldX: Int = newB.x
      val oldY: Int = newB.y

      newB.move(x, y)

      if (isRoomFor(newB)) {
        pool.insert(LinkedList(newB))
        model.declareDirty()
      }
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
    //FIXME: won't dirty the model as it is used, but could...

    def moveBlock(b: Block, x: Int, y: Int) = {
      val oldX: Int = b.x
      val oldY: Int = b.y

      b.move(x, y)

      if (isRoomFor(b))
        model.declareDirty()
      else
        b.move(oldX, oldY)
    }

    def rotateBlock(b: Block) = {
      b.rotate()

      model.declareDirty()
    }

    def isRoomFor(b: Block) = {
      pool.filter(x => x != b && x.intersects(b)).length == 0
    }

    def send(b: Block, ir: Dir) = {
      model.send(pool, b, ir)
    }

    private var cnt: Int = 0
    def draw(context: PApplet, lock: Lock, detailed: Boolean) = {
      lock.acquire
      model.computeConnections(pool)
      lock.release

      if (detailed)
        model.drawIRs(context)

      model.drawConnections(context)

      pool foreach { b =>
/*        if (cnt == 0)
          b.leds.foreach(row =>
            row.foreach(led =>
              led.randomColor()))*/
        b.draw(context, detailed)
      }

      cnt = (cnt + 1) % 10
    }
  }





}
