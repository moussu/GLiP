import java.io._
import java.net._
import scala.concurrent._
import simulator.view._
import scala.actors._

package simulator.ether {

  class Server(val pool: Pool, val lock: Lock) extends Actor {
    import Block._

    val port    = 4444
    val bufsize = 1024
    val buffer  = new Array[Byte](bufsize)
    val socket  = new DatagramSocket(port)
    val packet  = new DatagramPacket(buffer, bufsize)
    var portsToBlocks    = Map[Int, Block]()
    var blocksToPorts  = Map[Block, Int]()

    println ("UDP server started: localhost:" + port)

    def act() = {
      while (true) {
        socket.receive(packet)

        lock.acquire

        portsToBlocks.find(e => e._1 == packet.getPort()) match {
          case None => {
            val block = pool.addBlock()
            portsToBlocks += ((packet.getPort(), block))
            blocksToPorts += ((block, packet.getPort()))
            println("New client {port:" + packet.getPort() +
                    ", block:" + block + "}")
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
                socket.send(new DatagramPacket(data, data.size,
                                               InetAddress.getLocalHost,
                                               blocksToPorts(b)))
              case None => {}
            }
          }
          case 'l' => {
            val i = packetData(1) + 128
            val j = packetData(2) + 128
            val r = packetData(3) + 128
            val g = packetData(4) + 128
            val b = packetData(5) + 128
            portsToBlocks(packet.getPort()).leds(i)(j).setColor(r, g, b)
          }
        }

        lock.release
      }
    }

  }

}
