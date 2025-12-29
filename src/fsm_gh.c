/*
 * Git Helper FSM (ydjs)
 * ---------------------
 * Author: Jaehoon, 2025
 *
 * Implementation of the FSM, platform compatibility, and actions.
 */


#include "fsm_gh.h"
#include "env_loader.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* Gets git config value. Returns 1 if set, 0 if not set. Output stored in buffer. */
static int get_git_config(const char *key, char *buffer, size_t buffer_size) {
    char command[256];
    #ifdef _WIN32
        snprintf(command, sizeof(command), "git config --global --get %s 2>nul", key);
    #else
        snprintf(command, sizeof(command), "git config --global --get %s 2>/dev/null", key);
    #endif
    
    FILE *fp = POPEN(command, "r");
    if (!fp) return 0;
    
    if (fgets(buffer, buffer_size, fp)) {
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
            buffer[--len] = '\0';
        }
        PCLOSE(fp);
        return 1;
    }
    
    PCLOSE(fp);
    return 0;
}

/* Checks if git config is set. Returns 1 if set, 0 if not. */
static int is_git_config_set(const char *key) {
    char buffer[256];
    return get_git_config(key, buffer, sizeof(buffer));
}

/* Sets git credentials: unset existing, set new, configure helper */
static void set_git_credentials(const char *username, const char *email) {
    /* Unset existing */
    run_cmd("git config --global --unset user.name");
    run_cmd("git config --global --unset user.email");
    
    #ifdef _WIN32
        {
            char cred_path[512];
            const char *home = getenv("USERPROFILE");
            if (home) {
                snprintf(cred_path, sizeof(cred_path), "%s\\.git-credentials", home);
                if (ACCESS(cred_path) == 0) {
                    char cmd[1024];
                    snprintf(cmd, sizeof(cmd), "del /f \"%s\"", cred_path);
                    system(cmd);
                }
            }
        }
        run_cmd("git credential-cache exit");
    #else
        run_cmd("rm -f ~/.git-credentials");
        run_cmd("git credential-cache exit");
    #endif
    
    /* Set new credentials */
    run_cmd("git config --global user.name \"%s\"", username);
    run_cmd("git config --global user.email \"%s\"", email);
    run_cmd("git config --global --list");
}


/* * Generic Arrow Key Menu 
 * Returns the index of the selected option.
 */
static int show_menu(const char *title, const char *options[], int count) {
    int selected = 0;
    int key;

    while (1) {
        clear_screen();
        printf("Current branch: ");
        run_cmd("git branch --show-current");
        printf("\n");

        printf("=== %s ===\n\n", title);
        
        for (int i = 0; i < count; i++) {
            if (i == selected) {
                #ifdef _WIN32
                printf("  -> %s\n", options[i]);
                #else
                printf("\033[7m  -> %s \033[0m\n", options[i]);
                #endif
            } else {
                printf("     %s\n", options[i]);
            }
        }

        key = get_key();

        if (key == KEY_UP) {
            selected--;
            if (selected < 0) selected = count - 1;
        } else if (key == KEY_DOWN) {
            selected++;
            if (selected >= count) selected = 0;
        } else if (key == KEY_ENTER) {
            return selected;
        }
    }
}

/* --- LOGIC DEFINITIONS --- */

const char *SEMANTIC_TYPES[] = {
    "feat      - new user-facing feature",
    "fix       - bug fix",
    "refactor  - no behavior change",
    "perf      - performance improvement",
    "test      - add or update tests",
    "docs      - documentation only",
    "chore     - tooling, config, deps",
    "build     - build system changes",
    "ci        - CI/CD pipeline changes",
    "style     - formatting only",
    "revert    - revert previous change"
};

const char *SCOPES[] = {
    "auth", "api", "ui", "db", "cli", "build", "infra", "none"
};

/* --- ACTION HELPERS --- */
static void action_push(void);
static void action_fetch(void);
static void action_commit(void);
static void action_delete(void);


/* --- FSM STATES --- */

/* State -1: Exit */
int state_exit() {
    clear_screen();

    // normal edition
    printf("\n");
    printf("+===========================================================+\n");
    printf("|                                                           |\n");
    printf("|                GITHUB VERSION CONTROL FSM                 |\n");
    printf("|                     Version 1.2.4                         |\n");
    printf("|                                                           |\n");
    printf("|  Tool Name: vcs-gh                                        |\n");
    printf("|  Author:  Jaehoon Song                                    |\n");
    printf("|  Year:    2025                                            |\n");
    printf("|                                                           |\n");
    printf("|  A Finite State Machine CLI tool for                      |\n");
    printf("|  automating and linting Git/GitHub workflows              |\n");
    printf("|                                                           |\n");
    printf("+===========================================================+\n");
    printf("\n");
    lazyprintf("Good bye");
    printf("\n");
    printf("+===========================================================+\n");
    printf("|                                                           |\n");
    printf("|                   THANKS FOR USING vcs-gh                 |\n");
    printf("|   To contact the author: jsong421@gatech.edu              |\n");
    printf("|                                                           |\n");
    printf("+===========================================================+\n");
    pausef(NULL);

    // // nayun edition
    // printf("\n");
    // printf("+===========================================================+\n");
    // printf("|                                                           |\n");
    // printf("|                GITHUB VERSION CONTROL FSM                 |\n");
    // printf("|                     Version 1.2.7                         |\n");
    // printf("|                    (Nayun Edition)                        |\n");
    // printf("|                                                           |\n");
    // printf("|  Tool Name: vcs-gh-nayun                                  |\n");
    // printf("|  Author:  Jaehoon Song                                    |\n");
    // printf("|  Year:    2025                                            |\n");
    // printf("|                                                           |\n");
    // printf("|  A Finite State Machine CLI tool for                      |\n");
    // printf("|  automating and linting Git/GitHub workflows              |\n");
    // printf("|                                                           |\n");
    // printf("+===========================================================+\n");
    // printf("\n");
    // lazyprintf("Good bye");
    // printf("\n");
    // printf("+===========================================================+\n");
    // printf("|                                                           |\n");
    // printf("|             Dear Nayun, a great copywriter!               |\n");
    // printf("|                 Thank you for your help!                  |\n");
    // printf("|                                                           |\n");
    // printf("+===========================================================+\n");
    // pausef(NULL);


    return -99; /* Special code to stop the loop */
}

/* State 0: Start (Check Tools & Git Credentials) */
int state_start() {
    clear_screen();
    printf("Checking dependencies...\n");
    
    /* Check Git */
    int git_status;
    #ifdef _WIN32
        git_status = system("git --version > nul 2>&1");
    #else
        git_status = system("git --version > /dev/null 2>&1");
    #endif

    if (git_status != 0) {
        printf("Error: 'git' is not installed or not in PATH.\n");
        pausef(NULL);
        return -1;
    }

    /* Check Github CLI */
    int gh_status;
    #ifdef _WIN32
        gh_status = system("gh --version > nul 2>&1");
    #else
        gh_status = system("gh --version > /dev/null 2>&1");
    #endif

    if (gh_status != 0) {
        printf("Error: 'gh' (GitHub CLI) is not installed.\n");
        pausef(NULL);
        return -1;
    }

    /* Load .env file */
    if (load_dotenv(".env") != 0) {
        fprintf(stderr, "Warning: Failed to load .env\n");
    }

    /* Check if USERNAMES and EMAILS exist in .env */
    int username_count = 0, email_count = 0;
    char **usernames = get_env("USERNAMES", ";", &username_count);
    char **emails = get_env("EMAILS", ";", &email_count);

    /* Case 1: No .env info found - ask user to create .env */
    if (!usernames || username_count == 0 || !emails || email_count == 0) {
        clear_screen();
        printf("No USERNAMES and EMAILS found in .env file.\n");
        printf("Please provide git user information to create .env config.\n\n");
        
        char input[512];
        printf("Enter usernames (semicolon-separated, e.g., User1;User2;User3):\n");
        get_input_string(input, sizeof(input));
        
        if (strlen(input) == 0) {
            printf("No usernames provided. Exiting.\n");
            pausef(NULL);
            return -1;
        }
        
        char usernames_line[1024];
        snprintf(usernames_line, sizeof(usernames_line), "USERNAMES=\"%s\"\n", input);
        
        printf("Enter emails (semicolon-separated, e.g., user1@email.com;user2@email.com;user3@email.com):\n");
        get_input_string(input, sizeof(input));
        
        if (strlen(input) == 0) {
            printf("No emails provided. Exiting.\n");
            pausef(NULL);
            return -1;
        }
        
        char emails_line[1024];
        snprintf(emails_line, sizeof(emails_line), "EMAILS=\"%s\"\n", input);
        
        /* Append to .env file */
        FILE *f = fopen(".env", "a");
        if (f) {
            fprintf(f, "%s", usernames_line);
            fprintf(f, "%s", emails_line);
            fclose(f);
            printf("\n.env file updated with USERNAMES and EMAILS.\n");
            printf("The program will now exit. Restart to continue with git credential setup.\n");
        } else {
            fprintf(stderr, "Error: Could not write to .env file.\n");
            pausef(NULL);
            return -1;
        }
        
        pausef(NULL);
        return -1; /* Exit */
    }

    /* Validate: lengths must match */
    if (username_count != email_count) {
        clear_screen();
        printf("Error: Mismatch between USERNAMES (%d) and EMAILS (%d) count.\n", 
               username_count, email_count);
        printf("Please fix .env file.\n");
        free_env(usernames, username_count);
        free_env(emails, email_count);
        pausef(NULL);
        return -1;
    }

    /* Check if git config is set */
    int has_name = is_git_config_set("user.name");
    int has_email = is_git_config_set("user.email");

    /* Case 2: Git config not set - show menu to select credentials */
    if (!has_name || !has_email) {
        clear_screen();
        printf("Git global user.name or user.email is not set.\n");
        printf("Select credentials from .env:\n\n");
        
        /* Build menu options */
        char **menu_options = malloc(sizeof(char*) * (username_count + 1));
        for (int i = 0; i < username_count; i++) {
            menu_options[i] = malloc(512);
            snprintf(menu_options[i], 512, "%s <%s>", usernames[i], emails[i]);
        }
        
        int choice = show_menu("Select Git Credentials", (const char**)menu_options, username_count);
        
        /* Free menu options */
        for (int i = 0; i < username_count; i++) {
            free(menu_options[i]);
        }
        free(menu_options);
        
        /* Set selected credentials */
        printf("\nSetting git credentials...\n");
        set_git_credentials(usernames[choice], emails[choice]);
        
        printf("\nCredentials set successfully!\n");
        lazyprintf("Next: Checking if repository exists");
        pausef(NULL);
        
        free_env(usernames, username_count);
        free_env(emails, email_count);
        return 1; /* Move to State 1 */
    }

    /* Case 3: Git config is set - show existing and ask if want to change */
    clear_screen();
    printf("Current Git Global Configuration:\n");
    printf("-----------------------------------\n");
    run_cmd("git config --global --list");
    printf("-----------------------------------\n\n");
    
    printf("Do you want to change credentials? (y/n): ");
    char answer[8];
    get_input_string(answer, sizeof(answer));
    
    if (answer[0] == 'y' || answer[0] == 'Y') {
        clear_screen();
        printf("Select new credentials from .env:\n\n");
        
        /* Build menu options */
        char **menu_options = malloc(sizeof(char*) * (username_count + 1));
        for (int i = 0; i < username_count; i++) {
            menu_options[i] = malloc(512);
            snprintf(menu_options[i], 512, "%s <%s>", usernames[i], emails[i]);
        }
        
        int choice = show_menu("Select Git Credentials", (const char**)menu_options, username_count);
        
        /* Free menu options */
        for (int i = 0; i < username_count; i++) {
            free(menu_options[i]);
        }
        free(menu_options);
        
        /* Set selected credentials */
        printf("\nSetting git credentials...\n");
        set_git_credentials(usernames[choice], emails[choice]);
        
        printf("\nCredentials updated successfully!\n");
        lazyprintf("Next: Checking if repository exists");
        pausef(NULL);
    } else {
        printf("Keeping current credentials.\n");
        lazyprintf("Next: Checking if repository exists");
        pausef(NULL);
    }
    
    free_env(usernames, username_count);
    free_env(emails, email_count);
    
    return 1; /* Move to State 1 */
}

/* State 1: Check Repo Presence */
int state_check_repo() {
    /* Check if .git directory exists */
    if (ACCESS(".git") == 0) {
        /* .git exists - ask if user wants nested git repo */
        clear_screen();
        printf("Repository already initialized (.git exists).\n");
        printf("Do you want to create a nested git repository (inside .git)? (y/n, Enter=no): ");
        char answer[8];
        get_input_string(answer, sizeof(answer));
        
        /* If 'y'/'Y': proceed to initialization (nested repo) */
        if (answer[0] == 'y' || answer[0] == 'Y') {
            printf("Proceeding to initialization...\n");
            lazyprintf("Next: Initializing nested repository");
            pausef(NULL);
            return 2; /* Move to init */
        }
        
        /* If Enter (empty) or any other: skip to menu (normal flow) */
        printf("Skipping initialization.\n");
        lazyprintf("Next: Going to main menu");
        pausef(NULL);
        return 3; /* Move to menu */
    } else {
        /* .git does not exist, move to init */
        return 2;
    }
}

/* State 2: Initialize Repo */
int state_init() {
    /* Check for URLS and REPO_NAMES in .env */
    int url_count = 0, repo_name_count = 0;
    char **urls = get_env("URLS", ";", &url_count);
    char **repo_names = get_env("REPO_NAMES", ";", &repo_name_count);
    
    /* Case 1: URLS or REPO_NAMES missing or empty */
    if (!urls || url_count == 0 || !repo_names || repo_name_count == 0) {
        clear_screen();
        printf("Error: URLS and REPO_NAMES not found in .env file.\n");
        printf("Please add to .env:\n");
        printf("URLS=\"\"\n");
        printf("REPO_NAMES=\"\"\n");
        pausef(NULL);
        
        if (urls) free_env(urls, url_count);
        if (repo_names) free_env(repo_names, repo_name_count);
        return -1; /* Exit */
    }
    
    /* Case 2: Count mismatch */
    if (url_count != repo_name_count) {
        clear_screen();
        printf("Error: Mismatch between URLS (%d) and REPO_NAMES (%d) count.\n", 
               url_count, repo_name_count);
        printf("Please fix .env file so they have the same number of elements.\n");
        pausef(NULL);
        
        free_env(urls, url_count);
        free_env(repo_names, repo_name_count);
        return -1; /* Exit */
    }
    
    /* Case 3: Valid URLS and REPO_NAMES - check if already cloned */
    clear_screen();
    char cwd[1024];
    if (GET_CWD(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory: %s\n\n", cwd);
    } else {
        printf("Current directory: (error getting directory)\n\n");
    }
    
    /* Check if all repos are already cloned */
    int all_cloned = 1;
    for (int i = 0; i < url_count; i++) {
        if (ACCESS(repo_names[i]) != 0) {
            all_cloned = 0;
            break;
        }
    }
    
    if (all_cloned) {
        printf("All repositories are already initialized.\n");
        printf("Found %d repositories:\n", url_count);
        for (int i = 0; i < url_count; i++) {
            printf("  [%d] %s\n", i + 1, repo_names[i]);
        }
        lazyprintf("Next: Exiting");
        pausef(NULL);
        
        free_env(urls, url_count);
        free_env(repo_names, repo_name_count);
        return -1; /* Exit */
    }
    
    /* Some repos need to be cloned - show list and prompt */
    printf("Found %d repositories to clone:\n", url_count);
    for (int i = 0; i < url_count; i++) {
        int exists = (ACCESS(repo_names[i]) == 0);
        printf("  [%d] %s -> %s", i + 1, urls[i], repo_names[i]);
        if (exists) {
            printf(" (already exists)");
        }
        printf("\n");
    }
    
    printf("\nDo you want to clone all repositories to the current directory? (y/n): ");
    char answer[8];
    get_input_string(answer, sizeof(answer));
    
    if (answer[0] != 'y' && answer[0] != 'Y') {
        printf("Cloning cancelled.\n");
        free_env(urls, url_count);
        free_env(repo_names, repo_name_count);
        return -1; /* Exit */
    }
    
    /* Clone all repositories */
    clear_screen();
    printf("Cloning repositories...\n\n");
    for (int i = 0; i < url_count; i++) {
        /* Skip if already exists */
        if (ACCESS(repo_names[i]) == 0) {
            printf("[%d/%d] %s already exists, skipping...\n", i + 1, url_count, repo_names[i]);
            continue;
        }
        printf("[%d/%d] Cloning %s into %s...\n", i + 1, url_count, urls[i], repo_names[i]);
        run_cmd("git clone \"%s\" \"%s\"", urls[i], repo_names[i]);
        printf("\n");
    }
    
    printf("All repositories cloned successfully!\n");
    lazyprintf("Next: Exiting");
    pausef(NULL);
    
    free_env(urls, url_count);
    free_env(repo_names, repo_name_count);
    
    /* Exit after cloning */
    return -1;
}

/* Action: PUSH Flow */
static void action_push() {
    char branch[100];
    char title[200];
    char full_title[512];
    
    /* 1. Create Branch */
    clear_screen();
    printf("--- PUSH FLOW ---\n");
    printf("Enter new branch name (e.g., feature/login) or press Enter to go back to menu: ");
    get_input_string(branch, sizeof(branch));

    if (strlen(branch) == 0) {
        printf("Branch name is empty. Going back to menu.\n");
        lazyprintf("Next: Returning to main menu");
        pausef(NULL);
        return;
    }
    
    run_cmd("git checkout -b %s", branch);
    
    /* 2. Commit All Changes */
    run_cmd("git add .");
    
    /* 3. Semantic Selection */
    int type_idx = show_menu("Select Type", SEMANTIC_TYPES, 11);
    
    /* Extract just the first word from the selection (e.g. "feat") */
    char type_str[20];
    sscanf(SEMANTIC_TYPES[type_idx], "%s", type_str);

    int scope_idx = show_menu("Select Scope", SCOPES, 8);
    char *scope_str = (char*)SCOPES[scope_idx];

    clear_screen();
    printf("Type: %s\nScope: %s\n", type_str, scope_str);
    printf("Enter Title (e.g., add login button):\n");
    get_input_string(title, sizeof(title));

    /* Format: feat(auth): add login button */
    if (strcmp(scope_str, "none") == 0) {
        sprintf(full_title, "%s: %s", type_str, title);
    } else {
        sprintf(full_title, "%s(%s): %s", type_str, scope_str, title);
    }

    /* 4. Commit */
    run_cmd("git commit -m \"%s\"", full_title);

    /* 5. Push and PR */
    printf("\nPushing to remote...\n");
    run_cmd("git push --set-upstream origin %s", branch);
    
    printf("\nCreating Pull Request...\n");
    run_cmd("gh pr create --title \"%s\" --body \"Auto-generated PR by ydjs\"", full_title);
    
    printf("\nDone! Push and PR creation completed.\n");
    lazyprintf("Next: Returning to main menu");
    pausef(NULL);
}

/* Action: FETCH Flow */
static void action_fetch() {
    char input_buf[100];
    
    clear_screen();
    printf("--- FETCH FLOW ---\n");
    printf("Warning: This will hard reset local 'main' to match remote.\n");
    lazyprintf("Force-create '_cache_' at current state and save everything");
    run_cmd("git checkout -B _cache_");
    run_cmd("git add .");
    run_cmd("git commit -m \"_cache_\"");


    printf("Warning: This will delete all local branches except main/master/_cache_.\n");
    pausef(NULL);
    run_cmd("git fetch --all --prune");
    run_cmd("git branch | grep -v \"_cache_\" | xargs -r git branch -D");
    lazyprintf("Fetch complete.");
    /* Show branches */
    printf("\nRemote branches:\n");
    run_cmd("git branch -r");
    printf("\nLocal branches:\n");
    run_cmd("git branch");


    
    printf("\nEnter branch name without 'origin/' to checkout (or press Enter to set on origin/HEAD locally): ");
    get_input_string(input_buf, sizeof(input_buf));
    
    if (strlen(input_buf) > 0) {
        run_cmd("git checkout %s", input_buf);
        printf("Switched to branch: %s\n", input_buf);
    } else {
        run_cmd("echo $0");
        run_cmd("bash -c \"git checkout $(git symbolic-ref refs/remotes/origin/HEAD | sed 's|.*/||')\"");
        printf("Setting on HEAD.\n");
    }
    
    lazyprintf("Next: Returning to main menu");
    pausef(NULL);
}


/* Action: COMMIT Flow */
static void action_commit() {
    char msg[256];
    clear_screen();
    printf("--- QUICK COMMIT ---\n");
    printf("Staging all changes...\n");
    run_cmd("git add .");
    
    printf("Enter commit message:\n");
    get_input_string(msg, sizeof(msg));
    
    if (strlen(msg) > 0) {
        run_cmd("git commit -m \"%s\"", msg);
        printf("Committed..!\n");
        lazyprintf("Also, pushing to remote");
        run_cmd("git push origin HEAD");
        printf("Pushed to remote successfully.\n");
        lazyprintf("Next: Returning to main menu");
    } else {
        printf("Aborted (empty message).\n");
        lazyprintf("Next: Returning to main menu");
    }
    pausef(NULL);
}

/* Action: DELETE Flow */
static void action_delete() {
    char branch[100];
    char confirm[10];
    
    clear_screen();
    printf("--- DELETE BRANCH ---\n");
    run_cmd("git fetch --all --prune");
    run_cmd("git branch | grep -v \"_cache_\" | xargs -r git branch -D");
    run_cmd("git branch -r");
    printf("\nEnter a remote branch (without 'origin/') name to delete:\n");
    get_input_string(branch, sizeof(branch));
    
    if (strlen(branch) > 0) {
        printf("Are you sure you want to delete '%s'? (y/n)\n", branch);
        get_input_string(confirm, sizeof(confirm));
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            run_cmd("git push origin --delete %s", branch);
            printf("Deleted.\n");
        } else {
            printf("Cancelled.\n");
        }
    }
    lazyprintf("Next: Returning to main menu");
    pausef(NULL);
}

/* State 3: Main Menu */
int state_menu() {
    const char *options[] = {
        "Push   (Branch -> Commit -> PR)",
        "Fetch  (Reset Main -> Checkout)",
        "Exit",
        "Commit (Current Branch) - admin only",
        "Delete (Remove Branch) - admin only"
    };

    // length of options
    int option_count = sizeof(options) / sizeof(options[0]);

    int choice = show_menu("ydjs Git Helper", options, option_count);

    switch(choice) {
        case 0: action_push(); break;
        case 1: action_fetch(); break;
        case 2: return -1; /* Exit */
        case 3: action_commit(); break;
        case 4: action_delete(); break;
    }
    
    return 3; /* Loop back to menu */
}

