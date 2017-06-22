//OpenSCADA system module DAQ.FT3 file: BVTC.cpp
/***************************************************************************
 *   Copyright (C) 2011-2016 by Maxim Kochetkov                            *
 *   fido_max@inbox.ru                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/times.h>

#include "mod_FT3.h"
#include "BVTC.h"
//#include "da.h"

using namespace FT3;
//******************************************************
//* KA_BVTC                                             *
//******************************************************
KA_BVTC::KA_BVTC(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm, id), count_n(n), with_params(has_params), config(3 | (n << 4) | (3 << 10))
{
    mTypeFT3 = KA;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    loadIO(true);
    mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", ID);
}

DA::SLnk &KA_BVTC::lnk(int num)
{
    throw TError(mPrm.nodePath().c_str(),_("Link list is empty."));
}



KA_BVTC::~KA_BVTC()
{
//    data.clear();
}

string KA_BVTC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;
}

uint16_t KA_BVTC::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_BVTC_Error:
	    rc = BlckStateError;
	    break;
	case KA_BVTC_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_BVTC::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, 0, 0));
    return mPrm.owner().DoCmd(&Msg);

}

void KA_BVTC::tmHandler(void)
{
    NeedInit = false;
}

uint16_t KA_BVTC::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    l = 4;
	    break;
	case 2:
	    l = 2 + count_n * 2;
	    for(int j = 1; j <= count_n; j++) {
		//Value.Update(D[j * 2 + 1], tm);
	    }
	    break;
	}
	break;
    }
    return l;
}

uint8_t KA_BVTC::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    out[0] = 1;
	    l = 1;
	    break;
	case 1:
	    out[0] = config >> 8;
	    out[1] = config;
	    l = 2;
	    break;
	case 2:
	    for(uint8_t i = 0; i < count_n; i++) {
	//	out[i * 2] = data[i].Value.s;
	//	out[i * 2 + 1] = data[i].Value.vl;
	    }
	    l = count_n * 2;
	    break;
	}
    }
    return l;
}

uint8_t KA_BVTC::cmdSet(uint8_t * req, uint8_t addr)
{
    return 0;
}

uint16_t KA_BVTC::setVal(TVal &val)
{
    uint16_t rc = 0;
    return rc;
}

//---------------------------------------------------------------------------
//******************************************************
//* KA_TC                                             *
//******************************************************
KA_TC::KA_TC(TMdPrm& prm, DA &parent, uint16_t id, bool has_params) :
	DA(prm, id), parentDA(parent), with_params(has_params), config(0),
	Value(TSYS::strMess("TC_%d", id + 1), TSYS::strMess(_("TC %d"), id + 1)),
	Period(TSYS::strMess("Period_%d", id + 1), TSYS::strMess(_("Period TC %d"), id + 1)),
	Count(TSYS::strMess("Count_%d", id + 1), TSYS::strMess(_("Count TC %d"), id + 1))
{
    mTypeFT3 = KA;
    AddAttr(Value.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("0", ID + 1));
    if(with_params) {
	AddAttr(Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1", ID + 1));
	AddAttr(Count.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1", ID + 1));
    }

    loadIO(true);
}

KA_TC::~KA_TC()
{
    //data.clear();
}

string KA_TC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;
}

uint16_t KA_TC::GetState()
{
    //   tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    /*    Msg.L = 5;
     Msg.C = AddrReq;
     *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
     if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
     switch(mPrm.vlAt("state").at().getI(0, true)) {
     case KA_BVTC_Error:
     rc = BlckStateError;
     break;
     case KA_BVTC_Normal:
     rc = BlckStateNormal;
     break;
     }
     }*/
    return rc;
}

uint16_t KA_TC::PreInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    Msg.L += SerializeB(Msg.D + Msg.L, TC_DISABLED);
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_TC::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    if(Value.lnk.vlattr.at().getI(0, true) != TC_DISABLED) {
	Msg.L = 0;
	Msg.C = SetData;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
	Msg.L += Period.SerializeAttr(Msg.D + Msg.L);
	Msg.L += Count.SerializeAttr(Msg.D + Msg.L);
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't set channel %d", ID);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to set channel %d", ID);
	    }
	}
    }
    return rc;
}

uint16_t KA_TC::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 1));
    Msg.L += 3;
    rc = mPrm.owner().DoCmd(&Msg);
    if((rc == BAD2) || (rc == BAD3)) {
	throw TError(mPrm.nodePath().c_str(),_("Link list is empty."));
    } else {
	if(rc == ERROR) {
	    mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", ID);
	}
    }
    return rc;
}

uint16_t KA_TC::PostInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    loadVal(Value.lnk);
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    Msg.L += Value.SerializeAttr(Msg.D + Msg.L);
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_TC::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    return mPrm.owner().DoCmd(&Msg);
}

void KA_TC::loadIO(bool force)
{
//Load links
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    loadLnk(Value.lnk);
    loadLnk(Count.lnk);
    loadLnk(Period.lnk);

}

void KA_TC::saveIO(void)
{
//Save links
    saveLnk(Value.lnk);
    saveLnk(Count.lnk);
    saveLnk(Period.lnk);

}

void KA_TC::saveParam(void)
{
    saveVal(Value.lnk);
    if(with_params) {
	saveVal(Count.lnk);
	saveVal(Period.lnk);
    }
}

void KA_TC::loadParam(void)
{
    if(with_params) {
	loadVal(Period.lnk);
	loadVal(Count.lnk);
    }
}

void KA_TC::tmHandler(void)
{
    UpdateParam8(Value, PackID(parentDA.ID, ID, 0), 1);
    if(with_params) {
	UpdateParam28(Period, Count, PackID(parentDA.ID, ID, 1), 1);
    }
    NeedInit = false;
}

uint16_t KA_TC::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.k != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.n) {
    case 0:
	l = 4;
	Value.Update(D[3], tm);
	break;
    case 1:
	l = 5;
	if(with_params) {
	    Period.Update(D[3], tm);
	    Count.Update(D[4], tm);
	}
	break;
    }
    return l;
}

uint8_t KA_TC::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.k != ID) return 0;
    uint8_t l = 0;
    switch(ft3ID.n) {
    case 0:
	out[0] = Value.s;
	out[1] = Value.vl;
	l = 2;
	break;
    case 1:
	out[0] = Period.s;
	out[1] = Period.vl;
	out[2] = Count.vl;
	l = 3;
	break;
    }
    return l;
}

uint8_t KA_TC::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(req));
    if(ft3ID.g != ID) return 0;
    uint8_t l = 0;
    switch(ft3ID.n) {
    case 0:
	l = SetNew8Val(Value, addr, prmID, req[2]);
	break;
    case 1:
	l = SetNew28Val(Period, Count, addr, prmID, req[2], req[3]);
	break;
    }
    return l;
}

uint16_t KA_TC::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = ID;
    ft3ID.n = s2i(val.fld().reserve());
    ft3ID.g = parentDA.ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 0:
	Msg.L += Value.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    case 1:
	Msg.L += Period.SerializeAttr(Msg.D + Msg.L);
	Msg.L += Count.SerializeAttr(Msg.D + Msg.L);
	rc = 1;
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }

    return rc;
}

//---------------------------------------------------------------------------

B_BVTC::B_BVTC(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), count_n(n), ID(id), with_params(has_params)
{
    mTypeFT3 = GRS;
    blkID = 0x00;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

void B_BVTC::AddChannel(uint8_t iid)
{
    data.push_back(STCchannel(iid, this));
    AddAttr(data.back().Value.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1:%d", iid / 8));
    if(with_params) {
	AddAttr(data.back().Mask.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("2:%d", iid / 8));
    }
}

B_BVTC::~B_BVTC()
{
    data.clear();
}

string B_BVTC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}

void B_BVTC::loadIO(bool force)
{
//Load links
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Mask.lnk);
    }
}

void B_BVTC::saveIO()
{
//Save links
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Mask.lnk);
    }
}

void B_BVTC::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	uint8_t tmpval;
	uint8_t g = i / 8;
	if(data[i].Mask.vl == 0) {
	    tmpval = data[i].Value.Get();
	    if(tmpval != data[i].Value.vl) {
		data[i].Value.Update(tmpval);
		uint8_t E[1] = { 0 };
		for(int j = 0; j < 8; j++) {
		    E[0] |= (data[g * 8 + j].Value.vl & 0x01) << j;
		}
		PushInBE(1, 1, PackID(ID, 1, i / 8), E);
	    }
	}
	if(with_params) {
	    tmpval = data[i].Mask.Get();
	    if(tmpval != data[i].Mask.vl) {
		data[i].Mask.s = 0;
		data[i].Mask.Update(tmpval);
		uint8_t E[2] = { 0, 0 };
		for(int j = 0; j < 8; j++) {
		    E[1] |= (data[g * 8 + j].Mask.vl & 0x01) << j;
		}
		PushInBE(1, 2, PackID(ID, 2, i / 8), E);
	    }
	}
    }
    NeedInit = false;
}

uint16_t B_BVTC::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
	if(mPrm.owner().DoCmd(&Msg)) {
	    if(Msg.C == GOOD3) {
		uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
		Msg.L = 3 + nTC * 2;
		Msg.C = AddrReq;
		for(int i = 0; i < nTC; i++) {
		    *((uint16_t *) (Msg.D + i * 2)) = PackID(ID, 1, i); //TC Value
		}
		if(mPrm.owner().DoCmd(&Msg)) {
		    if(with_params) {
			Msg.L = 3 + nTC * 2;
			Msg.C = AddrReq;
			for(int i = 0; i < nTC; i++) {
			    *((uint16_t *) (Msg.D + i * 2)) = PackID(ID, 2, i); //маски ТC
			}
			if(mPrm.owner().DoCmd(&Msg)) {
			    rc = 1;
			}
		    } else {
			rc = 1;
		    }
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}

uint16_t B_BVTC::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3 + count_n / 4;
	    for(int j = 1; j <= count_n; j++) {
		data[j - 1].Value.Update((D[((j - 1) >> 3) + 3] >> (j % 8)) & 1, tm);
		if(with_params) {
		    data[j - 1].Mask.Update((D[((j - 1) >> 3) + 3 + count_n / 8] >> (j % 8)) & 1, tm);
		}
	    }
	    break;
	}
	break;
    case 1:
	l = 3;
	for(int i = 0; i < 8; i++) {
	    if((1 + (ft3ID.n << 3) + i) > count_n) break;
	    data[(ft3ID.n << 3) + i].Value.Update((D[2] >> i) & 1, tm);
	}
	break;
    case 2:
	l = 4;
	if(with_params) {
	    for(int i = 0; i < 8; i++) {
		if((1 + (ft3ID.n << 3) + i) > count_n) break;
		data[(ft3ID.n << 3) + i].Mask.Update((D[3] >> i) & 1, tm);
	    }
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 0 | blkID;
	    l = 1;
	    break;
	case 1:
	    out[0] = 0 | blkID;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i + 1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i + 1] |= (data[j].Value.vl & 0x01) << (j % 8);
		l++;
	    }
	    //mask;
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i + nTC + 1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i * 2 + 2] |= (data[j].Mask.vl & 0x01) << (j % 8);
		l++;
	    }
	    break;
	}
	break;

    case 1:
	//value
	out[0] = 0;
	if(ft3ID.n < nTC) {
	    for(uint8_t j = ft3ID.n * 8; j < (ft3ID.n + 1) * 8; j++) {
		out[0] |= (data[j].Value.vl & 0x01) << (j % 8);
	    }
	    l = 1;
	}
	break;
    case 2:
	//mask
	out[0] = 0;
	out[1] = 0;
	if(ft3ID.n < nTC) {
	    for(uint8_t j = ft3ID.n * 8; j < (ft3ID.n + 1) * 8; j++) {
		out[0] = data[j].Mask.s;
		out[1] |= (data[j].Mask.vl & 0x01) << (j % 8);
	    }
	    l = 2;
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;

    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch(ft3ID.k) {
    case 2:
	if(ft3ID.n < nTC) {
	    uint8_t newMask = req[2];
	    for(uint8_t i = ft3ID.n * 8; i < (ft3ID.n + 1) * 8; i++) {
		data[i].Mask.s = addr;
		if(data[i].Mask.lnk.Connected()) {
		    data[i].Mask.Set((uint8_t) (newMask & 0x01));
		    newMask = newMask >> 1;
		    l = 3;
		} else {
		    l = 0;
		    break;
		}

	    }
	    uint8_t E[2] = { addr, req[2] };
	    mPrm.owner().PushInBE(1, 2, prmID, E);
	}
	break;
    }
    return l;
}

uint16_t B_BVTC::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    uint16_t st = ft3ID.n * 8;
    uint16_t en = (ft3ID.n + 1) * 8;
    if(en > count_n) en = count_n;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    uint8_t mask = 0;
    for(int i = st; i < en; i++) {
	mask |= ((data[i].Mask.lnk.vlattr.at().getB(0, true)) << ((i - 1) % 8));
    }
    Msg.L += SerializeB(Msg.D + Msg.L, mask);
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
