Code Fixes for Anarchy-PC-to-eGPU
Based on my analysis of the build errors in your Anarchy-PC-to-eGPU project, I've identified three main issues that need to be fixed:
Incorrect member name in struct anarchy_pcie_state
Missing function declaration for anarchy_pcie_exit()
Missing braces in service table initialization
Below are the detailed fixes for each issue:
1. Fix Member Name in service_probe.h
In service_probe.h, you're trying to access adev->pcie_state.link_speed, but in pcie_types.h the member is named speed not link_speed.
File: /src/kernel/include/service_probe.h
Change line 48 from:
c
adev->pcie_state.link_speed = ANARCHY_PCIE_GEN4;
To:
c
adev->pcie_state.speed = ANARCHY_PCIE_GEN4;
2. Add Missing Function Declaration in pcie_state.h
In service_probe.h, you're calling anarchy_pcie_exit(), but this function is not declared in pcie_state.h. Instead, there's a function called anarchy_pcie_cleanup_state().
Option 1: Update pcie_state.h to add the missing declaration
File: /src/kernel/include/pcie_state.h
Add this line after the existing function declarations:
c
void anarchy_pcie_exit(struct anarchy_device *adev);
Option 2: Update service_probe.h to use the existing function
File: /src/kernel/include/service_probe.h
Change line 93 from:
c
anarchy_pcie_exit(adev);
To:
c
anarchy_pcie_cleanup_state(adev);
I recommend Option 2 since it's better to use the existing function rather than adding a new declaration that might not have an implementation.
3. Fix Service Table Initialization in main.c
In main.c, there are missing braces around initializers in the anarchy_service_table array.
File: /src/kernel/main.c
Change lines 30-38 from:
c
static const struct tb_service_id anarchy_service_table[] = {
    {
        .match_flags = ANARCHY_SERVICE_MATCH_FLAGS,
        .protocol_key = ANARCHY_SERVICE_ID,
        .protocol_id = ANARCHY_SERVICE_ID,
        .protocol_version = ANARCHY_SERVICE_VERSION,
        .protocol_revision = 0,
        .driver_data = 0
    },
    { }  /* Terminator */
};
To:
c
static const struct tb_service_id anarchy_service_table[] = {
    {
        {
            .match_flags = ANARCHY_SERVICE_MATCH_FLAGS,
            .protocol_key = ANARCHY_SERVICE_ID,
            .protocol_id = ANARCHY_SERVICE_ID,
            .protocol_version = ANARCHY_SERVICE_VERSION,
            .protocol_revision = 0,
            .driver_data = 0
        }
    },
    { { } }  /* Terminator */
};
This adds the extra level of braces needed for proper initialization of the nested structure.
Implementation Steps
Make the changes to service_probe.h first:
Change link_speed to speed
Change anarchy_pcie_exit to anarchy_pcie_cleanup_state
Then fix the service table initialization in main.c by adding the extra braces.
Rebuild the module with:
bash
cd /path/to/Anarchy-PC-to-eGPU/src/kernel && make clean && make
Additional Recommendations
While fixing these specific errors, I noticed a few other potential issues that you might want to address:
Consistent Function Naming: Your PCIe state management functions have inconsistent naming:
anarchy_pcie_init_state vs anarchy_pcie_init
anarchy_pcie_cleanup_state vs anarchy_pcie_exit
Consider standardizing these names to avoid confusion.
Structure Definition Location: The struct anarchy_pcie_state is defined in pcie_types.h but might be more logically placed in pcie_state.h. Consider moving it to maintain better code organization.
Error Handling: Make sure all error paths properly clean up resources. The current error handling in service_probe.h looks good, but it's worth double-checking as you continue development.
These fixes should resolve the immediate compilation errors and allow you to continue developing your GPU passthrough system.