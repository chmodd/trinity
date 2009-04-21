/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
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

#include "AggressorAI.h"
#include "Errors.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "VMapFactory.h"
#include "World.h"

#include <list>

int
AggressorAI::Permissible(const Creature *creature)
{
    // have some hostile factions, it will be selected by IsHostileTo check at MoveInLineOfSight
    if( !creature->isCivilian() && !creature->IsNeutralToAll() )
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

AggressorAI::AggressorAI(Creature *c) : CreatureAI(c), i_victimGuid(0), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

void AggressorAI::EnterEvadeMode()
{
    if( !m_creature->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his dead [guid=%u]", m_creature->GetGUIDLow());
        i_victimGuid = 0;
        m_creature->CombatStop();
        m_creature->DeleteThreatList();
        return;
    }

    Unit* victim = ObjectAccessor::GetUnit(*m_creature, i_victimGuid );

    if( !victim  )
    {
        DEBUG_LOG("Creature stopped attacking because victim is non exist [guid=%u]", m_creature->GetGUIDLow());
    }
    else if( !victim->isAlive() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is dead [guid=%u]", m_creature->GetGUIDLow());
    }
    else if( victim->HasStealthAura() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is stealth [guid=%u]", m_creature->GetGUIDLow());
    }
    else if( victim->isInFlight() )
    {
        DEBUG_LOG("Creature stopped attacking cuz his victim is fly away [guid=%u]", m_creature->GetGUIDLow());
    }
    else
    {
        DEBUG_LOG("Creature stopped attacking due to target out run him [guid=%u]", m_creature->GetGUIDLow());
        //i_state = STATE_LOOK_AT_VICTIM;
        //i_tracker.Reset(TIME_INTERVAL_LOOK);
    }

    if(!m_creature->GetCharmerOrOwner())
    {
        m_creature->RemoveAllAuras();

        // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
        if( m_creature->GetMotionMaster()->GetCurrentMovementGeneratorType() == TARGETED_MOTION_TYPE )
            m_creature->GetMotionMaster()->MoveTargetedHome();
    }
    else if (m_creature->GetOwner() && m_creature->GetOwner()->isAlive())
        m_creature->GetMotionMaster()->MoveFollow(m_creature->GetOwner(),PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);

    m_creature->DeleteThreatList();
    i_victimGuid = 0;
    m_creature->CombatStop();
    m_creature->SetLootRecipient(NULL);
}

void
AggressorAI::UpdateAI(const uint32 /*diff*/)
{
    // update i_victimGuid if m_creature->getVictim() !=0 and changed
    if(!UpdateVictim())
        return;

    i_victimGuid = m_creature->getVictim()->GetGUID();

    if( m_creature->isAttackReady() )
    {
        if( m_creature->IsWithinMeleeRange(m_creature->getVictim()))
        {
            m_creature->AttackerStateUpdate(m_creature->getVictim());
            m_creature->resetAttackTimer();
        }
    }
}

