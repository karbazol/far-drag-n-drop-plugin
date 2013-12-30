/**
 * @file utils.cpp
 * Contains misc utils.
 */
#include <windows.h>
#include <Aclapi.h>

/**
 * Allows to determine a base address of module containing specified moduleItem.
 * @param[in] moduleItem points to a symbol in the module which base address
 * should be found.
 * @return Address of module containing moduleItem.
 */
void* getModuleBaseAddress(void* moduleItem)
{
    MEMORY_BASIC_INFORMATION info;

    if (!VirtualQuery(moduleItem, &info, sizeof(info)))
        return 0;

    return info.AllocationBase;
}

static bool setSidDacl(PSECURITY_DESCRIPTOR pDescriptor, PSECURITY_DESCRIPTOR pSID)
{
    EXPLICIT_ACCESS ea= {0};
    PACL pACL;

    ea.grfAccessMode = SET_ACCESS;
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPTSTR) pSID;

    if(ERROR_SUCCESS == SetEntriesInAcl(1, &ea, NULL, &pACL) && pACL)
    {
        if(SetSecurityDescriptorDacl(pDescriptor, TRUE, pACL, FALSE))
        {
            return true;
        }
    }

    return false;
}

/**
 * Generates a security descriptor granting full access to Everyone.
 * Must be freed with freeEveryOneDescriptor.
 */
void* getEveryOneDescriptor()
{
    PSECURITY_DESCRIPTOR pDescriptor = {0};
    SID_IDENTIFIER_AUTHORITY Auth = SECURITY_WORLD_SID_AUTHORITY;
    PSECURITY_DESCRIPTOR pSID;

    if(AllocateAndInitializeSid(&Auth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSID))
    {
        pDescriptor = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
        if(pDescriptor)
        {
            if(InitializeSecurityDescriptor(pDescriptor, SECURITY_DESCRIPTOR_REVISION1))
            {
                if (setSidDacl(pDescriptor, pSID))
                {
                    return pDescriptor;
                }
            }
            free(pDescriptor);
            pDescriptor = NULL;
        }
    }
    
    return pDescriptor;    
}

/**
 * Frees a security descriptor allocated with getEveryOneDescriptor.
 */
void freeEveryOneDescriptor(void* p)
{
    free(p);
}

// vim: set et ts=4 ai :

