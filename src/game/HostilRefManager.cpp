/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "HostilRefManager.h"
#include "ThreatManager.h"
#include "Unit.h"
#include "DBCStructure.h"
#include "SpellMgr.h"

HostilRefManager::~HostilRefManager()
{
    deleteReferences();
}

//=================================================
// send threat to all my hateres for the pVictim
// The pVictim is hated than by them as well
// use for buffs and healing threat functionality

void HostilRefManager::threatAssist(Unit *pVictim, float fThreat, SpellEntry const *pThreatSpell, bool pSingleTarget)
{
    HostilReference* ref;

    float size = pSingleTarget ? 1.0f : getSize();            // if pSingleTarget do not divide threat
    ref = getFirst();
    while(ref != NULL)
    {
        float threat = ThreatCalcHelper::calcThreat(pVictim, iOwner, fThreat, (pThreatSpell ? GetSpellSchoolMask(pThreatSpell) : SPELL_SCHOOL_MASK_NORMAL), pThreatSpell);
        if(pVictim == getOwner())
            ref->addThreat(threat / size);          // It is faster to modify the threat durectly if possible
        else
            ref->getSource()->addThreat(pVictim, threat / size);
        ref = ref->next();
    }
}

//=================================================

void HostilRefManager::addTempThreat(float fThreat, bool apply)
{
    HostilReference* ref = getFirst();

    while(ref != NULL)
    {
        if (apply)
        {
            if (ref->getTempThreatModifier() == 0.0f)
                ref->addTempThreat(fThreat);
        }
        else
            ref->resetTempThreat();

        ref = ref->next();
    }
}


//=================================================

void HostilRefManager::addThreatPercent(int32 iPercent)
{
    HostilReference* ref;

    ref = getFirst();
    while (ref != NULL)
    {
        ref->addThreatPercent(iPercent);
        ref = ref->next();
    }
}

//=================================================
// The online / offline status is given to the method. The calculation has to be done before

void HostilRefManager::setOnlineOfflineState(bool bIsOnline)
{
    HostilReference* ref;

    ref = getFirst();
    while(ref != NULL)
    {
        ref->setOnlineOfflineState(bIsOnline);
        ref = ref->next();
    }
}

//=================================================
// The online / offline status is calculated and set

void HostilRefManager::updateThreatTables()
{
    HostilReference* ref = getFirst();
    while(ref)
    {
        ref->updateOnlineStatus();
        ref = ref->next();
    }
}

//=================================================
// The references are not needed anymore
// tell the source to remove them from the list and free the mem

void HostilRefManager::deleteReferences()
{
    HostilReference* ref = getFirst();
    while(ref)
    {
        HostilReference* nextRef = ref->next();
        ref->removeReference();
        delete ref;
        ref = nextRef;
    }
}

//=================================================
// delete one reference, defined by faction

void HostilRefManager::deleteReferencesForFaction(uint32 faction)
{
    HostilReference* ref = getFirst();
    while(ref)
    {
        HostilReference* nextRef = ref->next();
        if(ref->getSource()->getOwner()->getFactionTemplateEntry()->faction == faction)
        {
            ref->removeReference();
            delete ref;
        }
        ref = nextRef;
    }
}

//=================================================
// delete one reference, defined by Unit

void HostilRefManager::deleteReference(Unit *pCreature)
{
    HostilReference* ref = getFirst();
    while(ref)
    {
        HostilReference* nextRef = ref->next();
        if(ref->getSource()->getOwner() == pCreature)
        {
            ref->removeReference();
            delete ref;
            break;
        }
        ref = nextRef;
    }
}

//=================================================
// set state for one reference, defined by Unit

void HostilRefManager::setOnlineOfflineState(Unit *pCreature, bool bIsOnline)
{
    HostilReference* ref = getFirst();
    while(ref)
    {
        HostilReference* nextRef = ref->next();
        if(ref->getSource()->getOwner() == pCreature)
        {
            ref->setOnlineOfflineState(bIsOnline);
            break;
        }
        ref = nextRef;
    }
}

//=================================================

