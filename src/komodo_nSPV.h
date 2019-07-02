// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/******************************************************************************
 * Copyright © 2014-2019 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#ifndef KOMODO_NSPV_H
#define KOMODO_NSPV_H

#define NSPV_INFO 0x00
#define NSPV_INFORESP 0x01
#define NSPV_UTXOS 0x02
#define NSPV_UTXOSRESP 0x03

uint32_t CNode_lastinfo(CNode *pnode,uint32_t lastutxos);
uint32_t CNode_lastutxos(CNode *pnode,uint32_t lastutxos);

uint32_t NSPV_lastinfo,NSPV_lastutxos;

void komodo_nSPVreq(CNode *pfrom,std::vector<uint8_t> request) // received a request
{
    int32_t len; std::vector<uint8_t> response; uint32_t timestamp = time(NULL);
    if ( (len= request.size()) > 0 )
    {
        response.resize(1);
        if ( len == 1 && request[0] == NSPV_INFO ) // info
        {
            if ( timestamp > CNode_lastinfo(pfrom,0) + ASSETCHAINS_BLOCKTIME/2 )
            {
                response[0] = NSPV_INFORESP;
                CNode_lastinfo(pfrom,timestamp);
                pfrom->PushMessage("nSPV",response);
            }
        }
        else if ( request[0] == NSPV_UTXOS )
        {
            if ( timestamp > CNode_lastutxos(pfrom,0) + ASSETCHAINS_BLOCKTIME/2 )
            {
                response[0] = NSPV_UTXOSRESP;
                CNode_lastutxos(pfrom,timestamp);
                pfrom->PushMessage("nSPV",response);
            }
        }
    }
}

void komodo_nSPVresp(CNode *pfrom,std::vector<uint8_t> response) // received a response
{
    int32_t len;
    if ( (len= response.size()) > 0 )
    {
        switch ( response[0] )
        {
        case NSPV_INFORESP:
                fprintf(stderr,"got info response\n");
                break;
        case NSPV_UTXOSRESP:
                fprintf(stderr,"got utxos response\n");
                break;
        default: fprintf(stderr,"unexpected response %02x size.%d\n",response[0],(int32_t)response.size());
                break;
        }
    }
}

void komodo_nSPV(CNode *pto)
{
    std::vector<uint8_t> request; uint32_t timestamp = time(NULL);
    // limit frequency!
    if ( timestamp > NSPV_lastutxos + ASSETCHAINS_BLOCKTIME/2 )
    {
        if ( (pto->nServices & NODE_ADDRINDEX) != 0 && timestamp > CNode_lastutxos(pto,0) + ASSETCHAINS_BLOCKTIME )
        {
            // get utxo since lastheight
            if ( (rand() % 100) < 10 )
            {
                request.resize(1);
                request[0] = NSPV_UTXOS;
                NSPV_lastutxos = timestamp;
                CNode_lastutxos(pto,NSPV_lastutxos);
                pto->PushMessage("getnSPV",request);
                return;
            }
        }
    }
    if ( timestamp > NSPV_lastinfo + ASSETCHAINS_BLOCKTIME/2 && timestamp > CNode_lastinfo(pto,0) + ASSETCHAINS_BLOCKTIME )
    {
        if ( (rand() % 100) < 10 )
        {
            // query current height, blockhash, notarization info
            request.resize(1);
            request[0] = NSPV_INFO;
            NSPV_lastinfo = timestamp;
            CNode_lastinfo(pto,NSPV_lastinfo);
            pto->PushMessage("getnSPV",request);
        }
    }
}

#endif // KOMODO_NSPV_H