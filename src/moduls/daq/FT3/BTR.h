/***************************************************************************
 *   Copyright (C) 2011-2015 by Maxim Kochetkov                            *
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
#ifndef DA_BTR_H
#define DA_BTR_H

#include "da.h"

namespace FT3
{
    class B_BTR: public DA
    {
    public:
	//Methods
	B_BTR(TMdPrm& prm, uint16_t id, uint16_t nu, uint16_t nr, bool has_params);
	~B_BTR();
	uint16_t ID;
	uint16_t count_nu;
	uint16_t count_nr;
	bool with_params;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint8_t SetNewflVal(flData& d, uint8_t addr, uint16_t prmID, float val);
	uint16_t setVal(TVal &val);
	string getStatus(void);
	void saveIO(void);
	void loadIO(bool force = false );
	void tmHandler(void);
	class STRchannel
	{
	public:
	    STRchannel(uint8_t iid) :
		    id(iid), Value(TSYS::strMess("value_%d", id + 1).c_str(), TSYS::strMess(_("Value %d"), id + 1).c_str())
	    {
	    }
	    uint8_t id;

	    flData Value;
	};
	class STUchannel
	{
	public:
	    STUchannel(uint8_t iid) :
		    id(iid),
		    On(TSYS::strMess("on_%d", id + 1).c_str(), TSYS::strMess(_("On %d"), id + 1).c_str()),
		    Off(TSYS::strMess("off_%d", id + 1).c_str(), TSYS::strMess(_("Off %d"), id + 1).c_str()),
		    Run(TSYS::strMess("run_%d", id + 1).c_str(), TSYS::strMess(_("Run %d"), id + 1).c_str()),
		    Reset(TSYS::strMess("reset_%d", id + 1).c_str(), TSYS::strMess(_("Reset %d"), id + 1).c_str()),
	            Time(TSYS::strMess("time_%d", id + 1).c_str(), TSYS::strMess(_("Time %d"), id + 1).c_str()),
	            TC(TSYS::strMess("tc_%d", id + 1).c_str(), TSYS::strMess(_("TC %d"), id + 1).c_str()),
		    ExTime(TSYS::strMess("extime_%d", id + 1).c_str(), TSYS::strMess(_("ExTime %d"), id + 1).c_str())

	    {
	    }
	    uint8_t id;

	    ui8Data On, Off, Run, Reset, Time, TC, ExTime;
	};
	vector<STRchannel> TRdata;
	vector<STUchannel> TUdata;
	int lnkSize()
	{
	    return TUdata.size() * 4 + TRdata.size();
	}
	int lnkId(const string &id)
	{
	    for(int i_l = 0; i_l < TUdata.size(); i_l++) {
		if(TUdata[i_l].On.lnk.prmName == id) return i_l * 4;
		if(TUdata[i_l].Off.lnk.prmName == id) return i_l * 4 + 1;
		if(TUdata[i_l].Run.lnk.prmName == id) return i_l * 4 + 2;
		if(TUdata[i_l].Reset.lnk.prmName == id) return i_l * 4 + 3;
	    }

	    for(int i_l = 0; i_l < TRdata.size(); i_l++) {
		if(TRdata[i_l].Value.lnk.prmName == id) return i_l + TUdata.size() * 4;
	    }
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    if((TUdata.size() > 0) && ((TUdata.size() * 4) > num)) {
		switch(num % 4) {
		case 0:
		    return TUdata[num / 4].On.lnk;
		case 1:
		    return TUdata[num / 4].Off.lnk;
		case 2:
		    return TUdata[num / 4].Run.lnk;
		case 3:
		    return TUdata[num / 4].Reset.lnk;

		}
	    } else {
		return TRdata[num - TUdata.size() * 4].Value.lnk;
	    }

	}
    };

} //End namespace

#endif //DA_BTR_H