/* bzflag
* Copyright (c) 1993 - 2007 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "BufferedNetworkMessage.h"
#include "Pack.h"
//#include "GameKeeper.h"
//#include "bzfs.h"
//#include "bzfsMessages.h"
#include "NetHandler.h"
#include <algorithm>


// initialize the singleton
template <>
BufferedNetworkMessageManager* Singleton<BufferedNetworkMessageManager>::_instance = (BufferedNetworkMessageManager*)0;


BufferedNetworkMessage::BufferedNetworkMessage()
{
  data = NULL;
  dataSize = 0;
  packedSize = 0;
  readPoint = 0;

  code = 0;
  recipent = NULL;
  toAdmins = false;
}

BufferedNetworkMessage::BufferedNetworkMessage( const BufferedNetworkMessage &msg )
{
  packedSize = msg.packedSize;
  dataSize = msg.dataSize;
  data = (char*)malloc(dataSize);
  memcpy(data,msg.data,dataSize);
  code = msg.code;
  recipent = msg.recipent;
  toAdmins = msg.toAdmins;
}

BufferedNetworkMessage::~BufferedNetworkMessage()
{
  if (data)
    free(data);
}

void BufferedNetworkMessage::send ( NetHandler *to, uint16_t messageCode )
{
  recipent = to;
  code = messageCode;

  MSGMGR.queMessage(this);
}

void BufferedNetworkMessage::broadcast ( uint16_t messageCode, bool toAdminClients )
{
  recipent = NULL;
  code = messageCode;
  toAdmins = toAdminClients;
  MSGMGR.queMessage(this);
}

void BufferedNetworkMessage::packUByte( uint8_t val )
{
  checkData(sizeof(uint8_t));
  nboPackUByte(getWriteBuffer(),val);
  packedSize += sizeof(uint8_t);
}

void BufferedNetworkMessage::packShort( int16_t val )
{
  checkData(sizeof(int16_t));
  nboPackShort(getWriteBuffer(),val);
  packedSize += sizeof(int16_t);
}

void BufferedNetworkMessage::packInt( int32_t val )
{
  checkData(sizeof(int32_t));
  nboPackInt(getWriteBuffer(),val);
  packedSize += sizeof(int32_t);
}

void BufferedNetworkMessage::packUShort( uint16_t val )
{
  checkData(sizeof(uint16_t));
  nboPackUShort(getWriteBuffer(),val);
  packedSize += sizeof(uint16_t);
}

void BufferedNetworkMessage::packUInt( uint32_t val )
{
  checkData(sizeof(uint32_t));
  nboPackUInt(getWriteBuffer(),val);
  packedSize += sizeof(uint32_t);
}

void BufferedNetworkMessage::packFloat( float val )
{
  checkData(sizeof(float));
  nboPackFloat(getWriteBuffer(),val);
  packedSize += sizeof(float);
}

void BufferedNetworkMessage::packVector( const float* val )
{
  checkData(sizeof(float)*3);
  nboPackVector(getWriteBuffer(),val);
  packedSize += sizeof(float)*3;
}

void BufferedNetworkMessage::packString( const char* val, int len )
{
  checkData(len);
  nboPackString(getWriteBuffer(),val,len);
  packedSize += len;
}

void BufferedNetworkMessage::packStdString( const std::string& str )
{
  checkData(str.size()+sizeof(uint32_t));
  nboPackStdString(getWriteBuffer(),str);
  packedSize += str.size()+sizeof(uint32_t);
}

uint8_t BufferedNetworkMessage::unpackUByte( void )
{
  uint8_t v = 0;
  char *p = getReadBuffer();
  if (p)  
    packedSize += (char*)(nboUnpackUByte(p,v))-p;
  return v;
}

int16_t BufferedNetworkMessage::unpackShort( void )
{
  int16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackShort(p,v))-p;
  return v;
}

int32_t BufferedNetworkMessage::unpackInt( void )
{
  int32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackInt(p,v))-p;
  return v;
}

uint16_t BufferedNetworkMessage::unpackUShort( void )
{
  uint16_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUShort(p,v))-p;
  return v;
}

uint32_t BufferedNetworkMessage::unpackUInt( void )
{
  uint32_t v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackUInt(p,v))-p;
  return v;
}

float BufferedNetworkMessage::unpackFloat( void )
{
  float v = 0;
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackFloat(p,v))-p;
  return v;
}

float* BufferedNetworkMessage::unpackVector( float* val )
{
  memset(val,0,sizeof(float)*3);
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackVector(p,val))-p;
  return val;
}

const std::string& BufferedNetworkMessage::unpackStdString( std::string& str )
{
  char *p = getReadBuffer();
  if (p)
    packedSize += (char*)(nboUnpackStdString(p,str))-p;
  return str;
}

void BufferedNetworkMessage::addPackedData ( const char* d, size_t s )
{
  checkData(s);
  memcpy(getWriteBuffer(),d,s);
  packedSize += s;
}

void BufferedNetworkMessage::clear ( void )
{
  if (data)
    free(data);

  data = NULL;
  dataSize = 0;
  packedSize = 0;
  recipent = NULL;
  readPoint = 0;
  code = 0;
}

void BufferedNetworkMessage::reset ( void )
{
  readPoint = 0;
}

size_t BufferedNetworkMessage::size ( void )
{
  return packedSize;
}

bool BufferedNetworkMessage::process ( void )
{
  if (!data)
    checkData(4);

  NetworkMessageTransferCallback *transferCallback = MSGMGR.getTransferCallback();

  if (!transferCallback || !recipent && code == 0)
    return false;

  nboPackUShort(data, uint16_t(packedSize));
  nboPackUShort(data+sizeof(uint16_t), code);

  if (recipent)
  {
    return transferCallback->send(recipent, data, packedSize + 4) == packedSize+4;
  }
   
  // send message to everyone
  int mask = NetHandler::clientBZFlag;
  if (toAdmins)
    mask |= NetHandler::clientBZAdmin;
  transferCallback->broadcast(data, packedSize + 4, mask, code);

  return true;
}

void BufferedNetworkMessage::checkData ( size_t s )
{
  if ( packedSize + s > dataSize )
    data = reinterpret_cast<char*>(realloc(data, dataSize + ((int)ceil((s+4) / 256.0f)) * 256));
}

char* BufferedNetworkMessage::getWriteBuffer ( void )
{
  if (!data)
    return NULL;

  return data + 4 + packedSize;
}

char* BufferedNetworkMessage::getReadBuffer ( void )
{
  if (!data)
    return NULL;

  if (readPoint >= packedSize)
    return NULL;

  return data + 4 + readPoint;
}



//BufferedNetworkMessageManager
BufferedNetworkMessage* BufferedNetworkMessageManager::newMessage ( BufferedNetworkMessage* msgToCopy )
{
  BufferedNetworkMessage *msg = NULL;
  if (msgToCopy)
    msg = new BufferedNetworkMessage(*msgToCopy);
  else
    msg = new BufferedNetworkMessage;
  pendingOutgingMesages.push_back(msg);
  return msg;
}

size_t BufferedNetworkMessageManager::receiveMessages ( NetworkMessageTransferCallback *callback,  std::list<BufferedNetworkMessage*> &incomingMessages )
{
  BufferedNetworkMessage * msg = new BufferedNetworkMessage;

  while (callback->receive(msg))
    incomingMessages.push_back(msg);

  return incomingMessages.size();
}

void BufferedNetworkMessageManager::update ( void )
{
  if (transferCallback)
    sendPendingMessages();

  clearDeadIncomingMessages();
}

void BufferedNetworkMessageManager::clearDeadIncomingMessages ( void )
{
  MessageList::iterator itr = incomingMesages.begin();
  while (itr != incomingMesages.end() )
  {
    if ( !(*itr)->buffer() )
    {
      delete(*itr);
      incomingMesages.erase(itr++);
    }
    else
      itr++;
  }
}

void BufferedNetworkMessageManager::sendPendingMessages ( void )
{
  while ( outgoingQue.size() )
  {
    MessageDeque::iterator itr = outgoingQue.begin();
    if (*itr)
    {
      (*itr)->process();
      delete(*itr);
    }
    outgoingQue.pop_front();
  }
}

void BufferedNetworkMessageManager::queMessage ( BufferedNetworkMessage *msg )
{
  MessageList::iterator itr = std::find(pendingOutgingMesages.begin(),pendingOutgingMesages.end(),msg);
  if ( itr != pendingOutgingMesages.end() )
    pendingOutgingMesages.erase(itr);

  outgoingQue.push_back(msg);
}


void BufferedNetworkMessageManager::purgeMessages ( NetHandler *handler )
{
  MessageList::iterator itr = pendingOutgingMesages.begin();
  while ( itr != pendingOutgingMesages.end() )
  {
    if (handler == (*itr)->recipent)  // just kill the message and data, it'll be pulled from the list on the next update pass
    {
      delete(*itr);
      pendingOutgingMesages.erase(itr++);
    }
    else
      itr++;
  }

  MessageDeque::iterator qItr = outgoingQue.begin();
  while (qItr != outgoingQue.end())
  {
    if (handler == (*qItr)->recipent)  // just kill the message and data, it'll be pulled from the list on the next update pass
    {
      delete(*qItr);
      outgoingQue.erase(qItr++);
    }
    else
      qItr++;
  }
}

BufferedNetworkMessageManager::BufferedNetworkMessageManager()
{
  transferCallback = NULL;
}

BufferedNetworkMessageManager::~BufferedNetworkMessageManager()
{
  MessageList::iterator itr = pendingOutgingMesages.begin();
  while ( itr != pendingOutgingMesages.end() )
  {
      delete(*itr);
      itr++;
  }

  MessageDeque::iterator qItr = outgoingQue.begin();
  while (qItr != outgoingQue.end())
  {
      delete(*qItr);
      qItr++;
  }
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
