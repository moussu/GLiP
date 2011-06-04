import processing.core._
import spde.core._
import PConstants._
import scala.actors._
import scala.util.Random
import scala.collection.mutable._
import java.awt.Rectangle
import java.io._
import java.net._
import simulator.view._

package simulator.ether {

  class Server(val pool: Pool) extends Actor {
    import Block._

    val port    = 4444
    val bufsize = 1024
    val buffer  = new Array[Byte](bufsize)
    val socket  = new DatagramSocket(port)
    val packet  = new DatagramPacket(buffer, bufsize)
    var portsToBlocks    = Map[Int, Block]()
    var blocksToSockets  = Map[Block, DatagramSocket]()

    println ("UDP server started: localhost:" + port)

    def act() = {
      while (true) {
        socket.receive(packet)
        portsToBlocks.find(e => e._1 == packet.getPort()) match {
          case None => {
            val block = pool.addBlock()
            portsToBlocks   += ((packet.getPort(), block))
            blocksToSockets += ((block, new DatagramSocket(packet.getPort)))
          }
          case _ => ()
        }

        val packetData = packet.getData()

        packetData(0) match {
          case 'p' => {}
          case 's' => {
            val ir = Dir(packetData(1))
            def data: Array[Byte] = Array.ofDim(packetData.size - 2)
            Array.copy(packetData, 2, data, 0, packetData.size - 2)
            pool.send(portsToBlocks(packet.getPort()), ir) match {
              case Some(b) =>
                blocksToSockets(b).send(new DatagramPacket(data, data.size))
              case None => {}
            }
          }
          case 'l' => {
            val i = packetData(1)
            val j = packetData(2)
            val r = packetData(3)
            val g = packetData(4)
            val b = packetData(5)
            portsToBlocks(packet.getPort()).leds(i)(j).setColor(r, g, b)
          }
        }
      }
    }

  }

}
