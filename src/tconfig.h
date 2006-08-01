
//OpenSCADA system file: tconfig.h
/***************************************************************************
 *   Copyright (C) 2003-2006 by Roman Savochenko                           *
 *   rom_as@fromru.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
  
#ifndef TCONFIG_H
#define TCONFIG_H

#include <string>
#include <vector>

#include "tcntrnode.h"
#include "telem.h"

//Element type flags
#define FLD_NOVAL   0x10  //No value mirrored
#define FLD_KEY     0x20  //Primary key

using std::string;
using std::vector;

class TConfig;

class TCfg
{
    public:
	//Methods
	TCfg( TFld &fld, TConfig &owner );
	~TCfg();
	
	const string &name();
	
	bool operator==(TCfg &cfg);
        TCfg &operator=(TCfg &cfg);		
	
	bool  view( )		{ return m_view; }
	void  view( bool vw )	{ m_view = vw; }
	
	TFld &fld()		{ return *m_fld; }	
	
	//- Universal access -
        string getSEL( );
	string getS( );
	double getR( );
	int    getI( );
	bool   getB( );
	//- Direct access -
        string &getSd( );
        double &getRd( );
        int    &getId( );
        bool   &getBd( );					
	
	void setSEL( const string &val );
	void setS( const string &val );
	void setR( double val );
	void setI( int val );
	void setB( bool val );
	
    private:
	//Data
    	union 
	{
	    string *s_val;
	    double r_val;
	    int    i_val;
	    bool   b_val;
	}m_val;	
	
	//Attributes
	bool m_view;
	
	TFld     *m_fld;
	TConfig  &m_owner;	
};


class TTable;

class TConfig: public TValElem
{
    friend class TCfg;

    public:
	//Methods
	TConfig( TElem *Elements = NULL );
	~TConfig();

	TConfig &operator=(TConfig &cfg);

	void cfgList( vector<string> &list );
	TCfg &cfg( const string &n_val );
	
	void elem(TElem *Elements, bool first = false); 
	TElem &elem();

    protected:	
	//Methods
	virtual bool cfgChange( TCfg &cfg )	{ return true; }
	
	void cntrCmdMake( XMLNode *fld, const char *path, int pos );
        void cntrCmdProc( XMLNode *fld, const string &elem );
	
    private:
	//Methods
	void detElem( TElem *el );
	void addFld( TElem *el, unsigned id );
	void delFld( TElem *el, unsigned id );	
	
	//Attributes
	vector<TCfg*>	value;
	TElem   	*m_elem;
        bool     	single;
};

#endif // TCONFIG_H
