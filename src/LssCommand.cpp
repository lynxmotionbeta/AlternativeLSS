//
// Created by guru on 9/25/21.
//

#include "LssCommand.h"

namespace lss {
namespace command {

const char* to_string(ID cmd) {
  switch(cmd) {
  case DBLT: return "DBLT";
  case DBCL: return "DBCL";
  case QSLOT: return "QSLOT";
  case SLOT: return "SLOT";
  case QSLOTCOUNT: return "QSLOTCOUNT";
  case SLOTCOUNT: return "SLOTCOUNT";
  case PROTOCOL: return "PROTOCOL";
  case QPROTOCOL: return "QPROTOCOL";
  case DBPT: return "DBPT";
  case CAS: return "CAS";
  case CAH: return "CAH";
  case CHD: return "CHD";
  case CAA: return "CAA";
  case CAD: return "CAD";
  case CSD: return "CSD";
  case CSR: return "CSR";
  case CRC: return "CRC";
  case CB: return "CB";
  case CID: return "CID";
  case CG: return "CG";
  case CPO: return "CPO";
  case CLE: return "CLE";
  case CLP: return "CLP";
  case CLN: return "CLN";
  case CO: return "CO";
  case CAR: return "CAR";
  case CEM: return "CEM";
  case CFPC: return "CFPC";
  case CLED: return "CLED";
  case CLB: return "CLB";
  case REC: return "REC";
  case REV: return "REV";
  case RET: return "RET";
  case CHM: return "CHM";
  case CFD: return "CFD";
  case CIS: return "CIS";
  case CRIS: return "CRIS";
  case DEFAULT: return "DEFAULT";
  case UPDATE: return "UPDATE";
  case CONFIRM: return "CONFIRM";
  case ABR: return "ABR";
  case CABR: return "CABR";
  case DBMMGV: return "DBMMGV";
  case DBDW: return "DBDW";
  case DBCW: return "DBCW";
  case DBCP: return "DBCP";
  case DBBT: return "DBBT";
  case DBLC: return "DBLC";
  case SD: return "SD";
  case SR: return "SR";
  case S: return "S";
  case T: return "T";
  case CH: return "CH";
  case CL: return "CL";
  case D: return "D";
  case P: return "P";
  case M: return "M";
  case MD: return "MD";
  case RDM: return "RDM";
  case H: return "H";
  case L: return "L";
  case WD: return "WD";
  case WR: return "WR";
  case HM: return "HM";
  case MMD: return "MMD";
  case FACTORYOFFSET: return "FACTORYOFFSET";
  case CN: return "CN";
  case CNH: return "CNH";
  case Q: return "Q";
  case QMD: return "QMD";
  case QCR: return "QCR";
  case QEM: return "QEM";
  case QP: return "QP";
  case QO: return "QO";
  case QD: return "QD";
  case QDT: return "QDT";
  case QAA: return "QAA";
  case QAD: return "QAD";
  case QAS: return "QAS";
  case QAH: return "QAH";
  case QHD: return "QHD";
  case QWD: return "QWD";
  case QS: return "QS";
  case QWR: return "QWR";
  case QSD: return "QSD";
  case QSR: return "QSR";
  case QVT: return "QVT";
  case QTQ: return "QTQ";
  case QTQT: return "QTQT";
  case QTQM: return "QTQM";
  case QC: return "QC";
  case QV: return "QV";
  case QT: return "QT";
  case QTC: return "QTC";
  case QM: return "QM";
  case QMS: return "QMS";
  case QF: return "QF";
  case QN: return "QN";
  case QB: return "QB";
  case QID: return "QID";
  case QG: return "QG";
  case QFD: return "QFD";
  case QIS: return "QIS";
  case QRIS: return "QRIS";
  case QY: return "QY";
  case QAR: return "QAR";
  case QFPC: return "QFPC";
  case QFACTORYOFFSET: return "QFACTORYOFFSET";
  case QPO: return "QPO";
  case QLE: return "QLE";
  case QLP: return "QLP";
  case QLN: return "QLN";
  case QLED: return "QLED";
  case QLB: return "QLB";
  case QRP: return "QRP";
  case QCSL: return "QCSL";
  case QMMD: return "QMMD";
  case QIPE: return "QIPE";
  case QABR: return "QABR";
  case LED: return "LED";
  case SCSL: return "SCSL";
  case AA: return "AA";
  case AD: return "AD";
  case AS: return "AS";
  case AH: return "AH";
  case EM: return "EM";
  case CGM: return "CGM";
  case G: return "G";
  case O: return "O";
  case AR: return "AR";
  case FPC: return "FPC";
  case Y: return "Y";
  case ECR: return "ECR";
  case EQR: return "EQR";
  case RESET: return "RESET";
  case IPE: return "IPE";
  case GCT: return "GCT";
  default: return nullptr;
  }
}

} // ns:Command
} // ns:Lss

