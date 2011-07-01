import java.io._
import java.net._
import scala.concurrent._
import simulator.view._
import scala.actors._

package simulator.ether {

  class Server(val pool: Pool, val lock: Lock) extends Actor {
    import Block._

    val port    = 4444
    val bufsize = 65535
    val buffer  = new Array[Byte](bufsize)
    val socket  = new DatagramSocket(port)
    val packet  = new DatagramPacket(buffer, bufsize)
    var portsToBlocks  = Map[Int, Block]()
    var blocksToPorts  = Map[Block, Int]()

    println ("UDP server started: localhost:" + port)

    def act() = {
      while (true) {
        socket.receive(packet)

        lock.acquire

        val packetCmd = packet.getData.head
        val packetData: Array[Byte] = Array.ofDim(packet.getLength - 1)
        Array.copy(packet.getData, 1, packetData, 0, packet.getLength - 1)
        val packetDataInt = packetData map { x => x.asInstanceOf[Int] + 128 }

        packetCmd match {
          case 'p' => {
            val answerPort = new String(packetData).toInt
            val receivePort = packet.getPort
            val block = pool.addBlock()
            val msg = "p" + block.id.toString

            println("New client {" +
                    "answerPort:"  + answerPort  + ", " +
                    "receivePort:" + receivePort + ", " +
                    "block:"       + block       + "}")

            portsToBlocks = portsToBlocks.updated(receivePort, block)
            blocksToPorts = blocksToPorts.updated(block,       answerPort)

            println("Sending: " + msg);

            socket.send(new DatagramPacket(msg.getBytes, msg.size,
                                           InetAddress.getLocalHost,
                                           answerPort))
            lock.release
            pool.refresh
          }
          case 'm' => {
            val data = packetData.tail
            val from = portsToBlocks(packet.getPort())
            val from_iface = Dir.fromChar(packetData(0).toChar)
            pool.send(from, from_iface) match {
              case Some((to, to_iface)) => {
                val msg = Array(to_iface.toString.charAt(0).toByte) ++ data
                println(from + ":" + from_iface.toString +
                        " -> " + to_iface.toString + ":" + to)
                socket.send(new DatagramPacket(msg, msg.size,
                                               InetAddress.getLocalHost,
                                               blocksToPorts(to)))
              }
              case None => {}
            }
            lock.release
          }
          case 'l' => {
            for (i <- 0 until 8) {
              for (j <- 0 until 8) {
                val r = packetDataInt(i * 8 * 3 + j * 3 + 0)
                val g = packetDataInt(i * 8 * 3 + j * 3 + 1)
                val b = packetDataInt(i * 8 * 3 + j * 3 + 2)
                portsToBlocks(packet.getPort()).leds(i)(j).setColor(r, g, b)
              }
            }
            lock.release
          }
          case _ => lock.release
        }
      }
    }

  }

}
