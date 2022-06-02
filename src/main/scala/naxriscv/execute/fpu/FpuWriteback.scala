package naxriscv.execute.fpu

import naxriscv.{Global, ROB}
import naxriscv.interfaces.{CommitService, CsrService, DecoderService, RegfileService, RobService, WakeRegFile, WakeRegFileService, WakeRob, WakeRobService}
import naxriscv.misc.RegFilePlugin
import naxriscv.riscv.{CSR, FloatRegFile}
import naxriscv.utilities.Plugin
import spinal.core._
import spinal.lib._

class FpuWriteback extends Plugin  with WakeRobService with WakeRegFileService{

  override def wakeRobs    = List(logic.get.wakeRob)
  override def wakeRegFile = List(logic.get.wakeRf)


  val setup = create early new Area{
    val floatCompletion = Flow(FpuFloatCompletion(ROB.ID_WIDTH, 32 + Global.RVD.get.toInt*32))
    val rob = getService[RobService]
    val decoder = getService[DecoderService]
    val rf = findService[RegfileService](_.rfSpec == FloatRegFile)
    val floatWrite = rf.newWrite(false, 0)
    val completion = rob.newRobCompletion()
    val csr = getService[CsrService]

    val unschedule = out Bool()

    rob.retain()
    csr.retain()
  }

  val logic = create late new Area{
    val s = setup.get
    import s._


    val flags = Reg(FpuFlags()) //TODO
    flags.NV init(False) //setWhen(port.completion.fire && port.completion.flags.NV)
    flags.DZ init(False) //setWhen(port.completion.fire && port.completion.flags.DZ)
    flags.OF init(False) //setWhen(port.completion.fire && port.completion.flags.OF)
    flags.UF init(False) //setWhen(port.completion.fire && port.completion.flags.UF)
    flags.NX init(False) //setWhen(port.completion.fire && port.completion.flags.NX)

    val rm = Reg(Bits(3 bits)) init(0)
    csr.readWrite(CSR.FCSR,   5 -> rm)
    csr.readWrite(CSR.FCSR,   0 -> flags)
    csr.readWrite(CSR.FRM,    0 -> rm)
    csr.readWrite(CSR.FFLAGS, 0 -> flags)

    setup.unschedule := RegNext(getService[CommitService].reschedulingPort(true).valid) init(False)

    val physical = rob.readAsyncSingle(decoder.PHYS_RD, floatCompletion.robId)

    val wakeRob = Flow(WakeRob())
    wakeRob.valid := floatCompletion.fire
    wakeRob.robId := floatCompletion.robId

    val wakeRf = Flow(WakeRegFile(decoder.REGFILE_RD, decoder.PHYS_RD, needBypass = true))
    wakeRf.valid    := floatCompletion.fire
    wakeRf.physical := physical
    wakeRf.regfile  := decoder.REGFILE_RD.rfToId(FloatRegFile)

    floatWrite.valid := floatCompletion.fire
    floatWrite.robId := floatCompletion.robId
    floatWrite.data := floatCompletion.value
    floatWrite.address := physical

    completion.valid := floatCompletion.fire
    completion.id := floatCompletion.robId

    rob.release()
    csr.release()
  }
}
