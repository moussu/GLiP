import sbt._
import spde._

class Simulator(info: ProjectInfo) extends DefaultSpdeProject(info) {
  override def fork = Some(new ForkScalaRun {
        override def runJVMOptions = super.runJVMOptions ++ Seq("-Xmx512m")
        override def scalaJars = Seq(buildLibraryJar.asFile, buildCompilerJar.asFile)
  })
}
