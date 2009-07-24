
//OpenSCADA system module Special.FLibSYS file: varchfnc.cpp
/***************************************************************************
 *   Copyright (C) 2009 by Roman Savochenko                                *
 *   rom_as@fromru.com                                                     *
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

#include "varchfnc.h"

using namespace FLibSYS;

//*************************************************
//* VArchObj - Value archive object               *
//*************************************************
VArchObj::VArchObj( ) : mIsArch(false), mBuf(NULL)
{

}

VArchObj::~VArchObj( )
{
    close();
}

bool VArchObj::open( const string &inm )
{
    close();

    try
    {
	AutoHD<TVArchive> ta;
	if( dynamic_cast<TVal*>(&SYS->nodeAt(inm,0,'.').at()) )
	    ta = dynamic_cast<TVal&>(SYS->nodeAt(inm,0,'.').at()).arch();
	else if( dynamic_cast<TVArchive*>(&SYS->nodeAt(inm,0,'.').at()) )
	    ta = SYS->nodeAt(inm,0,'.');
	if( ta.freeStat() ) return false;
	mArch = new AutoHD<TVArchive>(ta);
	mIsArch = true;
    }catch(TError err)	{ return false; }

    return true;
}

bool VArchObj::open( TFld::Type vtp, int isz, int ipr, bool ihgrd, bool ihres )
{
    close();
    
    mBuf = new TValBuf(vtp,isz,ipr,ihgrd,ihres);
    if( !mBuf )	return false;
    mIsArch = false;

    return true;
}

void VArchObj::close( )
{
    if( isArch() && mArch ) delete mArch;
    if( !isArch() && mBuf ) delete mBuf;
    mIsArch = false;
    mBuf = NULL;
}

AutoHD<TVArchive> VArchObj::arch( )
{
    if( !isArch() || !mArch ) return AutoHD<TVArchive>();
    return *mArch;    
}

TValBuf *VArchObj::buf( )
{
    if( isArch() ) return NULL;
    return mBuf;
}

TVariant VArchObj::propGet( const string &id )
{
    throw TError("VArchObj",_("Properties no supported by object."));
}

void VArchObj::propSet( const string &id, TVariant val )
{
    throw TError("VArchObj",_("Properties no supported by object."));
}

TVariant VArchObj::funcCall( const string &id, vector<TVariant> &prms )
{
    if( id == "begin" )
    {
	long long vbg;
	if( isArch() )	vbg = arch().at().begin( (prms.size()>=2) ? prms[1].getS() : "" );
	else
	{
	    if( !buf() ) return EVAL_INT;
	    vbg = buf()->begin();
	}
	if( prms.size() >= 1 ) { prms[0].setI(vbg%1000000); prms[0].setModify(); }
	return (int)(vbg/1000000);
    }
    if( id == "end" )
    {
	long long vbg;
	if( isArch() )	vbg = arch().at().end( (prms.size()>=2) ? prms[1].getS() : "" );
	else
	{
	    if( !buf() ) return EVAL_INT;
	    vbg = buf()->end();
	}
	if( prms.size() >= 1 ) { prms[0].setI(vbg%1000000); prms[0].setModify(); }
	return (int)(vbg/1000000);
    }
    if( id == "get" && prms.size() >= 2 )
    {
	TVariant vl;
	long long vtm = (long long)prms[0].getI()*1000000 + prms[1].getI();
	if( isArch() ) vl = arch().at().getVal( &vtm, (prms.size()>=3)?prms[2].getB():false, (prms.size()>=4)?prms[3].getS():"" );
	else
	{
	    if( !buf() ) return EVAL_REAL;
	    switch(buf()->valType())
	    {
		case TFld::Boolean:	vl = buf()->getB( &vtm, (prms.size()>=3)?prms[2].getB():false );	break;
		case TFld::Integer:	vl = buf()->getI( &vtm, (prms.size()>=3)?prms[2].getB():false );	break;
		case TFld::Real:	vl = buf()->getR( &vtm, (prms.size()>=3)?prms[2].getB():false );	break;
		case TFld::String:	vl = buf()->getS( &vtm, (prms.size()>=3)?prms[2].getB():false );	break;
	    }
	}
	prms[0].setI(vtm/1000000); prms[0].setModify();
	prms[1].setI(vtm%1000000); prms[1].setModify();
	return vl;
    }
    if( id == "set" && prms.size() >= 3 )
    {
	if( isArch() )
	    switch( arch().at().valType() )
	    {
		case TFld::Boolean:	arch().at().setB(prms[0].getB(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::Integer:	arch().at().setI(prms[0].getI(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::Real:	arch().at().setR(prms[0].getR(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::String:	arch().at().setS(prms[0].getS(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
	    }
	else
	{
	    if( !buf() ) return false;
	    switch(buf()->valType())
	    {
		case TFld::Boolean:	buf()->setB(prms[0].getB(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::Integer:	buf()->setI(prms[0].getI(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::Real:	buf()->setR(prms[0].getR(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
		case TFld::String:	buf()->setS(prms[0].getS(),(long long)prms[1].getI()*1000000+prms[2].getI());	break;
	    }
	}
	return true;
    }
    if( id == "copy" && prms.size() >= 5 ) 
    {
	VArchObj *src = dynamic_cast<VArchObj*>(prms[0].getO());
	if( !src ) return false;
        
	AutoHD<TVArchive> tarch;
	if( src->isArch() )
	{
	    TValBuf* vb = NULL;
	    if( isArch() )
	    {
		tarch = arch();
		vb = &tarch.at();
	    }
	    else vb = buf();
	    if( !vb )     return false;
	    src->arch().at().getVals( *vb, (long long)prms[1].getI()*1000000+prms[2].getI(),
					   (long long)prms[3].getI()*1000000+prms[4].getI(), (prms.size()>=6)?prms[5].getS():"" );
	}
	else if( isArch() )
	{
	    TValBuf* vb = NULL;
	    if( src->isArch() )
	    {
		tarch = src->arch();
		vb = &tarch.at();
	    }
	    else vb = src->buf();
	    if(!vb)     return false;
	    arch().at().setVals( *vb, (long long)prms[1].getI()*1000000+prms[2].getI(),
				      (long long)prms[3].getI()*1000000+prms[4].getI(), (prms.size()>=6)?prms[5].getS():"" );
	}
	else
	{
	    TValBuf* svb = src->buf();
	    TValBuf* dvb = buf();
	    if( !svb || !dvb ) return false;
	    svb->getVals( *dvb, (long long)prms[1].getI()*1000000+prms[2].getI(), (long long)prms[3].getI()*1000000+prms[4].getI() );
	}
	return true;
    }
    throw TError("VArchObj",_("Function '%s' error or not enough parameters."),id.c_str());
}
