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

        lock.release

        pool.refresh()

        lock.acquire

        val packetCmd = packet.getData.head
        val packetData: Array[Byte] = Array.ofDim(packet.getLength - 1)
        Array.copy(packet.getData, 1, packetData, 0, packet.getLength - 1)
        val packetDataInt = packetData map { x => x.asInstanceOf[Int] + 128 }

        packetCmd match {
          case 'p' => {}
          case 'm' => {
            val data = packetData.tail
            val ir = Dir.fromChar(packetData(0).toChar)
            pool.send(portsToBlocks(packet.getPort()), ir) match {
              case Some((b, d)) => {
                val msg = Array(d.toString.charAt(0).toByte) ++ data
                socket.send(new DatagramPacket(msg, msg.size,
                                               InetAddress.getLocalHost,
                                               blocksToPorts(b)))
              }
              case None => {}
            }
          }
          case 'l' => {
            val i = packetDataInt(0)
            val j = packetDataInt(1)
            val r = packetDataInt(2)
            val g = packetDataInt(3)
            val b = packetDataInt(4)
            portsToBlocks(packet.getPort()).leds(i)(j).setColor(r, g, b)
          }
        }

        lock.release
      }
    }

  }

}
