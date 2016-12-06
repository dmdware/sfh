













#include "trigger.h"
#include "effect.h"
#include "../save/savemap.h"
#include "../net/net.h"
#include "../net/netconn.h"

Trigger* g_scripthead = NULL;

Trigger::Trigger()
{
	strcpy(name, "");
	prev = NULL;
	next = NULL;
	enabled = true;
}

int32_t CountTriggers()
{
	int32_t numt = 0;
	Trigger* t = g_scripthead;
    
	while(t)
	{
		numt++;
		t = t->next;
	}
    
	return numt;
}

//return true if map switched
bool Trigger::checkallmet()
{
	for(int32_t i=0; i<conditions.size(); i++)
	{
		Condition* c = &conditions[i];
        
		if(!c->met)
			return false;
	}
    
	return execute();
}

void Trigger::resetconditions()
{
	for(int32_t i=0; i<conditions.size(); i++)
	{
		Condition* c = &conditions[i];
		c->met = false;
	}
}

//return true if map switched
bool Trigger::execute()
{
	for(int32_t i=0; i<effects.size(); i++)
	{
		Effect* e = &effects[i];
        char fullpath[1024];
        
		switch(e->type)
		{
            case EFFECT_DISABTRIGGER:
                if(e->trigger != NULL)
                    e->trigger->enabled = false;
                break;
            case EFFECT_ENABTRIGGER:
                if(e->trigger != NULL)
                {
                    e->trigger->enabled = true;
                    if(e->trigger->checkallmet())
                        return true;
                }
                break;
            case EFFECT_EXECUTETRIGGER:
                if(e->trigger != NULL)
                {
                    e->trigger->execute();
                }
                break;
            case EFFECT_MESSAGE:
                GMessage(e->textval.c_str());
                break;
            case EFFECT_MESSAGEIMG:
                GMessageG(e->textval.c_str(), e->imgdw, e->imgdh);
                break;
            case EFFECT_RESETCONDITIONS:
                if(e->trigger != NULL)
                    e->trigger->resetconditions();
                break;
            case EFFECT_LOSS:
                FreeMap();
                EndSess();
                return true;
            case EFFECT_VICTORY:
                FullWritePath(e->textval.c_str(), fullpath);
                //LoadMap(e->textval.c_str());
                LoadMap(fullpath);
                return true;
            default: break;
		}
	}
    
	return false;
}

//returns true if map switched
bool ConditionMet(int32_t type, int32_t target, float fval, int32_t count, int32_t owner)
{
	Trigger* t = g_scripthead;
    
	std::vector<Trigger*> tocheck;
    
	while(t)
	{
		if(t->enabled)
		{
			for(int32_t i=0; i<t->conditions.size(); i++)
			{
				Condition* c = &t->conditions[i];
                
				if(c->met)
					continue;
                
				if(c->type != type)
					continue;
                
				switch(type)
				{
                    case CONDITION_ONFINISHEDB:
                        if(c->target == target)
                            goto conditionmatch;
                        break;
                    case CONDITION_ONPLACEB:
                        if(c->target == target)
                            goto conditionmatch;
                        break;
                    case CONDITION_ONBUILDU:
                        if(c->target == target)
                            goto conditionmatch;
                        break;
                    case CONDITION_ONPLACEOTHERB:
                        if(c->target != target)
                            goto conditionmatch;
                        break;
                    case CONDITION_CAPTUREB:
                        if(c->target == target)
                            goto conditionmatch;
                        break;
                    case CONDITION_MESSAGECLOSED:
                    case CONDITION_ONLOADMAP:
                    case CONDITION_ALLCONSTRUCTIONFINI:
                    case CONDITION_ALLROADACCESSIBLE:
                    case CONDITION_ROADINACCESSABILITY:
                        goto conditionmatch;
                        break;
                    case CONDITION_PRICESET:
                        if(c->target == target && c->fval == fval)
                            goto conditionmatch;
                        break;
                    default: break;
				}
                
				continue;
                
            conditionmatch:
				c->met = true;
                
				// execute only after all triggers' conditions have been checked
				tocheck.push_back(t);
			}
		}
        
		t = t->next;
	}
    
	for(int32_t i=0; i<tocheck.size(); i++)
	{
		if(tocheck[i]->checkallmet())	//returns true if map switched
			return true;
	}
    
	return false;
}

void Change_NewTrigger(int32_t dummy)
{
}

Trigger* GetTrigger(int32_t which)
{
	if(which < 0)
		return NULL;
    
	Trigger* curs = g_scripthead;
	int32_t curi = 0;
    
	while(curs)
	{
		if(curi == which)
			return curs;
        
		curi ++;
		curs = curs->next;
	}
    
	return NULL;
}

int32_t TriggerID(Trigger* trigger)
{
	int32_t cur = 0;
	Trigger* curt = g_scripthead;
    
	while(curt)
	{
		if(curt == trigger)
			return cur;
        
		curt = curt->next;
		cur++;
	}
    
	return -1;
}

Trigger* GetScriptTail()
{
	if(g_scripthead == NULL)
		return NULL;
    
	Trigger* thise = g_scripthead;
    
	while(true)
	{
		if(thise->next != NULL)
			thise = thise->next;
		else
			return thise;
	}
    
	return NULL;
}

void FreeScript()
{
	Trigger* thise = GetScriptTail();
    
	if(thise == NULL)
		return;
    
	while(true)
	{
		if(thise->prev != NULL && thise != NULL)
		{
			thise = thise->prev;
            if(thise->next != NULL)
                delete thise->next;
			thise->next = NULL;
		}
		else
		{
            if(thise != NULL)
                delete thise;
			break;
		}
	}
    /*
    Trigger* thise = g_scripthead;
    
    while(thise)
    {
        Trigger* preve = thise;
        thise = thise->next;
        
        for(int32_t i=0; i<preve->effects.size(); i++)
        {
            NSLog(@"effect %d: %s", i, preve->effects[i].textval.c_str());
        }
        
        delete preve;
    }
    */
	g_scripthead = NULL;
}

bool UniqueTrigger(const char* name)
{
	return false;	//TODO
}
