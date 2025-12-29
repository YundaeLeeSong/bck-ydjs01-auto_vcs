/*
 * Git Helper FSM (ydjs)
 * ---------------------
 * Author: Jaehoon, 2025
 *
 * A Finite State Machine (FSM) CLI tool to automate Git/GitHub workflows.
 * Features:
 * - Cross-platform Arrow Key Menus (Windows/Linux/macOS)
 * - Semantic Commit Builder (feat/fix/chore...)
 * - Automated PR Creation (gh cli)
 * - Branch Cleanup
 *
 * Compile:
 * (Windows): make
 * (Linux/Mac): make
 */


#include "fsm_gh.h"
#include "report.h"
#include "env_loader.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>



/* --- MAIN ENTRY --- */
int main(int argc, char *argv[]) {
    /* --- ENVIRONMENT REPORT --- */
    print_environment_report(argc, argv);
    
    /* --- ENVIRONMENT VARIABLE LOAD --- */
    printf("\n=== ENVIRONMENT VARIABLE LOAD ===\n\n");
    if (load_dotenv(".env") != 0) {
        fprintf(stderr, "Failed to load .env\n");
        /* proceed: maybe env vars are set externally */
    }

    // int db_count = 0;
    // char **db_array = get_env("DB_HOST", ";", &db_count);
    // if (db_array && db_count > 0) {
    //     printf("DB_HOST=%s\n", db_array[0]);
    //     free_env(db_array, db_count);
    // } else {
    //     printf("DB_HOST=(null)\n");
    // }
    
    // int port_count = 0;
    // char **port_array = get_env("DB_PORT", ";", &port_count);
    // if (port_array && port_count > 0) {
    //     printf("DB_PORT=%s\n", port_array[0]);
    //     free_env(port_array, port_count);
    // } else {
    //     printf("DB_PORT=(null)\n");
    // }





    // int count = 0;
    // /* Split by space " " */
    // char **roles = get_env("SERVER_ROLES", " ", &count);

    // if (roles) {
    //     printf("Found %d roles:\n", count);
    //     for (int i = 0; i < count; i++) printf(" - Role[%d]: %s\n", i, roles[i]);
        
    //     /* Clean up memory when done */
    //     free_env(roles, count);
    // } else {
    //     printf("SERVER_ROLES not found or empty.\n");
    // }

    // /* Split by comma */
    // char **admins = get_env("SERVER_ADMINS", ",", &count);
    
    // for(int i=0; i<count; i++) {
    //     /* Output will be clean: "alice", "bob", "charlie" */
    //     printf("Admin: '%s'\n", admins[i]);
    // }
    // free_env(admins, count);


  
    // printf("\n=== END OF REPORT ===\n");
    lazyprintf("Next: Starting Git Helper FSM");
    
    pausef(NULL);
    
    /* --- MAIN LOGIC --- */
    int current_state = 0;

    #ifndef _WIN32
    enable_raw_mode();
    #endif

    while (current_state != -99) {
        switch (current_state) {
            case -1:    current_state = state_exit(); break;
            case 0:     current_state = state_start(); break;
            case 1:     current_state = state_check_repo(); break;
            case 2:     current_state = state_init(); break;
            case 3:     current_state = state_menu(); break;
            default:    current_state = -1; break;                  // Fail-safe
        }
    }

    #ifndef _WIN32
    disable_raw_mode();
    #endif

    return 0;
}