#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX 100

struct User {
    char username[30];
    char password[30];
    char role[10];
};

struct Medicine {
    int id;
    char name[50];
    char company[50];
    char category[30];
    int quantity;
    float price;
    char expiry[15];
};

struct SaleLog {
    char customer[50];
    char medicine[50];
    int quantity;
    float total;
    char date[20];
};


void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void getInput(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL)
        buffer[strcspn(buffer, "\n")] = '\0';
}

int caseInsensitiveStrstr(const char *haystack, const char *needle) {
    char h[MAX], n[MAX];
    int i;
    for (i = 0; haystack[i] && i < MAX-1; i++)
        h[i] = tolower(haystack[i]);
    h[i] = 0;
    for (i = 0; needle[i] && i < MAX-1; i++)
        n[i] = tolower(needle[i]);
    n[i] = 0;
    return strstr(h, n) != NULL;
}


void registerUser();
void login();
void logout();
void changePassword(const char username[]);
void logSession(const char username[], const char mode[]);
int isAdmin();
int isStaff();
void addMedicine();
void updateMedicine();
void deleteMedicine();
void searchMedicine();
void sortMedicines();
void checkLowStock();
void checkExpiry();
void billing();
void salesReport();
void backup();
void restore();
int getUniqueMedicineID();

char currentUser[30] = "";
char currentRole[10] = "";




int main() {
    int choice;
    printf("\n--- Welcome to the Medical Store System ---\n");
    printf("1. Login\n2. Register\nEnter choice: ");
    if (scanf("%d", &choice) != 1) {
        clearInputBuffer();
        printf("Invalid input!\n");
        return 1;
    }
    clearInputBuffer();

    if (choice == 1) {
        login();
    } else if (choice == 2) {
        registerUser();
        login();
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    //login();

    while (1) {
        printf("\n||------- Medical Store System Menu -------||\n");
        if (isAdmin()) {
            printf("1. Add Medicine\n2. Update Medicine\n3. Delete Medicine\n4. Search Medicine\n");
            printf("5. Sort Medicines\n6. Billing\n7. Sales Report\n8. Check Low Stock\n9. Check Expiry\n");
            printf("10. Change Password\n11. Backup\n12. Restore\n13. Logout\n");
        } else if (isStaff()) {
            printf("1. Search Medicine\n2. Sort Medicines\n3. Billing\n4. Check Low Stock\n");
            printf("5. Change Password\n6. Logout\n");
        } else {
            printf("Role not recognized. Exiting.\n");
            exit(1);
        }

        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        clearInputBuffer();

        if (isAdmin()) {
            switch (choice) {
                case 1: addMedicine(); break;
                case 2: updateMedicine(); break;
                case 3: deleteMedicine(); break;
                case 4: searchMedicine(); break;
                case 5: sortMedicines(); break;
                case 6: billing(); break;
                case 7: salesReport(); break;
                case 8: checkLowStock(); break;
                case 9: checkExpiry(); break;
                case 10: changePassword(currentUser); break;
                case 11: backup(); break;
                case 12: restore(); break;
                case 13: logout(); exit(0);
                default: printf("Invalid choice.\n");
            }
        } else if (isStaff()) {
            switch (choice) {
                case 1: searchMedicine(); break;
                case 2: sortMedicines(); break;
                case 3: billing(); break;
                case 4: checkLowStock(); break;
                case 5: changePassword(currentUser); break;
                case 6: logout(); exit(0);
                default: printf("Invalid choice.\n");
            }
        }
    }
    return 0;
}


//User Manual

void registerUser() {
    FILE *fp = fopen("use.txt", "a");
    struct User u;

    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    printf("\n--- Register [First Time] ---\n");
    printf("Enter username: ");
    scanf("%29s", u.username);
    clearInputBuffer();
    printf("Enter password: ");
    scanf("%29s", u.password);
    clearInputBuffer();

    do {
        printf("Enter role (admin/staff): ");
        scanf("%9s", u.role);
        clearInputBuffer();
    } while (strcmp(u.role, "admin") != 0 && strcmp(u.role, "staff") != 0);

    fprintf(fp, "%s %s %s\n", u.username, u.password, u.role);

    fclose(fp);
    printf("User registered successfully!\n");
}

void login() {
    FILE *fp;
    struct User u;
    char uname[30], pass[30];
    int found = 0;

    fp = fopen("use.txt", "r");
    if (!fp || fgetc(fp) == EOF) {
        if (fp) fclose(fp);
        registerUser();
    } else {
        fclose(fp);
    }

    while (!found) {
        fp = fopen("use.txt", "r");
        if (!fp) {
            printf("Error opening user file.\n");
            exit(1);
        }

        printf("\nLogin\nUsername: ");
        scanf("%29s", uname);
        clearInputBuffer();
        printf("Password: ");
        scanf("%29s", pass);
        clearInputBuffer();

        while (fscanf(fp, "%29s %29s %9s", u.username, u.password, u.role) == 3) {
            if (strcmp(u.username, uname) == 0 && strcmp(u.password, pass) == 0) {
                strcpy(currentUser, uname);
                strcpy(currentRole, u.role);
                logSession(uname, "login");
                printf("Login successful!\n");
                found = 1;
                break;
            }
        }
        fclose(fp);

        if (!found) {
            printf("Invalid username or password. Try again.\n");
        }
    }
}

void logout() {
    logSession(currentUser, "logout");
    printf("Logged out successfully.\n");
}

void logSession(const char username[], const char mode[]) {
    FILE *fp = fopen("session_log.txt", "a");
    if (!fp) return;
    time_t now = time(NULL);
    fprintf(fp, "%s %s at %s", username, mode, ctime(&now));
    fclose(fp);
}

void changePassword(const char username[]) {
    FILE *fp = fopen("use.txt", "r");
    struct User users[100];
    int count = 0, found = 0;
    char newPass[30];

    if (!fp) {
        printf("User file not found.\n");
        return;
    }

    while (fscanf(fp, "%29s %29s %9s", users[count].username, users[count].password, users[count].role) == 3) {
        count++;
    }
    fclose(fp);

    printf("Enter new password (no space): ");
    scanf("%29s", newPass);
    clearInputBuffer();

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            strcpy(users[i].password, newPass);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("User not found.\n");
        return;
    }

    fp = fopen("use.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %s %s\n", users[i].username, users[i].password, users[i].role);
    }
    fclose(fp);

    printf("Password updated successfully.\n");
}

int isAdmin() {
    return strcmp(currentRole, "admin") == 0;
}

int isStaff() {
    return strcmp(currentRole, "staff") == 0;
}


// MEDICINE SYSTEM

int getUniqueMedicineID() {
    int id;
    FILE *fp = fopen("medicines.txt", "r");
    struct Medicine med;
    int unique = 1;
    do {
        unique = 1;
        printf("Enter Medicine ID: ");
        if (scanf("%d", &id) != 1) {
            clearInputBuffer();
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        clearInputBuffer();
        if (fp) {
            rewind(fp);
            while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                          &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry) == 7) {
                if (med.id == id) {
                    unique = 0;
                    printf("This ID already exists! Try another.\n");
                    break;
                }
            }
        }
    } while (!unique);
    if (fp) fclose(fp);
    return id;
}

void addMedicine() {
    FILE *fp = fopen("medicines.txt", "a");
    struct Medicine med;

    if (!fp) {
        printf("Error opening medicine file.\n");
        return;
    }
    printf("\n<<<< Add Medicine >>>>\n");
    med.id = getUniqueMedicineID();

    getInput("Medicine Name: ", med.name, sizeof(med.name));
    getInput("Company: ", med.company, sizeof(med.company));

    printf("Price: ");
    while (scanf("%f", &med.price) != 1) {
        clearInputBuffer();
        printf("Invalid input! Enter Price: ");
    }
    clearInputBuffer();

    getInput("Category: ", med.category, sizeof(med.category));

    printf("Quantity: ");
    while (scanf("%d", &med.quantity) != 1) {
        clearInputBuffer();
        printf("Invalid input! Enter Quantity: ");
    }
    clearInputBuffer();

    getInput("Expiry Date (dd-mm-yyyy): ", med.expiry, sizeof(med.expiry));

    fprintf(fp, "%d|%s|%s|%.2f|%s|%d|%s\n",
            med.id, med.name, med.company, med.price, med.category, med.quantity, med.expiry);

    fclose(fp);
    printf("Medicine added successfully!\n");
}

void updateMedicine() {
    FILE *fp = fopen("medicines.txt", "r");
    struct Medicine meds[MAX];
    int count = 0, id, found = 0;

    if (!fp) {
        printf("Error opening file.\n");
        return;
    }

    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                  &meds[count].id, meds[count].name, meds[count].company, &meds[count].price,
                  meds[count].category, &meds[count].quantity, meds[count].expiry) == 7 && count < MAX) {
        count++;
    }
    fclose(fp);

    printf("\nMedicine ID to Update: ");
    if (scanf("%d", &id) != 1) {
        clearInputBuffer();
        printf("Invalid input!\n");
        return;
    }
    clearInputBuffer();

    for (int i = 0; i < count; i++) {
        if (meds[i].id == id) {
            printf("New Quantity: ");
            while (scanf("%d", &meds[i].quantity) != 1) {
                clearInputBuffer();
                printf("Invalid input! Enter Quantity: ");
            }
            clearInputBuffer();

            printf("New Price: ");
            while (scanf("%f", &meds[i].price) != 1) {
                clearInputBuffer();
                printf("Invalid input! Enter Price: ");
            }
            clearInputBuffer();

            found = 1;
            break;
        }
    }

    fp = fopen("medicines.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%s|%.2f|%s|%d|%s\n",
                meds[i].id, meds[i].name, meds[i].company, meds[i].price,
                meds[i].category, meds[i].quantity, meds[i].expiry);
    }
    fclose(fp);

    if (found) {
        printf("Medicine updated successfully!\n");
    } else {
        printf("Medicine not found.\n");
    }
}

void deleteMedicine() {
    FILE *fp = fopen("medicines.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    struct Medicine med;
    int id, found = 0, choice;

    if (!fp || !temp) {
        printf("Error opening file.\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    printf("Medicine ID to Delete: ");
    if (scanf("%d", &id) != 1) {
        clearInputBuffer();
        printf("Invalid input!\n");
        fclose(fp);
        fclose(temp);
        return;
    }
    clearInputBuffer();

    printf("Are you sure you want to remove?\n1. NO\n2. YES\nChoice: ");
    if (scanf("%d", &choice) != 1) {
        clearInputBuffer();
        printf("Invalid input!\n");
        fclose(fp);
        fclose(temp);
        return;
    }
    clearInputBuffer();
    if (choice != 2) {
        fclose(fp);
        fclose(temp);
        return;
    }

    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                  &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry) == 7) {
        if (med.id != id) {
            fprintf(temp, "%d|%s|%s|%.2f|%s|%d|%s\n",
                    med.id, med.name, med.company, med.price, med.category, med.quantity, med.expiry);
        } else {
            found = 1;
        }
    }

    fclose(fp);
    fclose(temp);

    remove("medicines.txt");
    rename("temp.txt", "medicines.txt");

    if (found) {
        printf("Medicine deleted successfully!\n");
    } else {
        printf("Medicine not found.\n");
    }
}

void searchMedicine() {
    FILE *fp = fopen("medicines.txt", "r");
    struct Medicine med;
    char match[50];
    int found = 0;
    if (!fp) {
        printf("Error opening file.\n");
        return;
    }
    getInput("Search Medicine Name: ", match, sizeof(match));
    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
        &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry) == 7) {
        if (caseInsensitiveStrstr(med.name, match)) {
            printf("\nID: %d\nName: %s\nCompany: %s\nCategory: %s\nQuantity: %d\nPrice: %.2f\nExpiry: %s\n",
                   med.id, med.name, med.company, med.category, med.quantity, med.price, med.expiry);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No medicine found!\n");
}

void sortMedicines() {
    FILE *fp = fopen("medicines.txt", "r");
    if (!fp) {
        printf("Error opening file.\n");
        return;
    }

    struct Medicine med[MAX], temp;
    int i = 0, j, n;

    while (i < MAX && fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                             &med[i].id, med[i].name, med[i].company, &med[i].price, med[i].category, &med[i].quantity, med[i].expiry) == 7) {
        i++;
    }
    fclose(fp);
    n = i;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (strcmp(med[j].name, med[j + 1].name) > 0) {
                temp = med[j];
                med[j] = med[j + 1];
                med[j + 1] = temp;
            }
        }
    }

    printf("\nMedicines are Sorted:\n");
    for (i = 0; i < n; i++) {
        printf("ID: %d || Name: %s || Qty: %d || Price: %.2f || Expiry: %s\n",
               med[i].id, med[i].name, med[i].quantity, med[i].price, med[i].expiry);
    }
}

void checkLowStock() {
    FILE *fp = fopen("medicines.txt", "r");
    struct Medicine med;
    int limit, hasLow = 0;

    if (!fp) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter stock level limit: ");
    if (scanf("%d", &limit) != 1) {
        clearInputBuffer();
        printf("Invalid input!\n");
        fclose(fp);
        return;
    }
    clearInputBuffer();

    printf("\n>>> Low Stock Medicines <<<\n");
    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                  &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry) == 7) {
        if (med.quantity < limit) {
            printf("ID: %d || Name: %s || Quantity: %d\n",
                   med.id, med.name, med.quantity);
            hasLow = 1;
        }
    }
    fclose(fp);

    if (!hasLow) printf("No low stock medicine found.\n");
}

void checkExpiry() {
    FILE *fp = fopen("medicines.txt", "r");
    struct Medicine med;
    char today[15];
    int dd, mm, yyyy;
    int found = 0;

    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    getInput("Today's Date (dd-mm-yyyy): ", today, sizeof(today));
    if (sscanf(today, "%d-%d-%d", &dd, &mm, &yyyy) != 3) {
        printf("Invalid date format.\n");
        fclose(fp);
        return;
    }

    printf("\n---- Expired Medicines ----\n");

    while (fscanf(fp, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]\n",
                  &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry) == 7) {
        int ed, em, ey;
        if (sscanf(med.expiry, "%d-%d-%d", &ed, &em, &ey) != 3) continue;
        if (ey < yyyy || (ey == yyyy && em < mm) || (ey == yyyy && em == mm && ed <= dd)) {
            printf("ID: %d | Name: %s | Expiry: %s\n", med.id, med.name, med.expiry);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No expired medicine found.\n");
}

void billing() {
    FILE *fp = fopen("medicines.txt", "r");
    FILE *temp = fopen("temp_medicines.txt", "w");
    FILE *sales = fopen("sales.txt", "a");
    struct Medicine med;
    struct SaleLog s;
    int id, qty, found = 0;
    char line[256];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (fp == NULL || temp == NULL || sales == NULL) {
        printf("Error opening file(s).\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        if (sales) fclose(sales);
        return;
    }

    getInput("Customer Name: ", s.customer, sizeof(s.customer));

    printf("Medicine ID for sell: ");
    while (scanf("%d", &id) != 1) {
        printf("Enter valid number!\n");
        clearInputBuffer();
    }
    clearInputBuffer();

    printf("Quantity: ");
    while (scanf("%d", &qty) != 1) {
        printf("Enter valid number!\n");
        clearInputBuffer();
    }
    clearInputBuffer();

    while (fgets(line, sizeof(line), fp)) {
        int res = sscanf(line, "%d|%49[^|]|%49[^|]|%f|%29[^|]|%d|%14[^\n]",
            &med.id, med.name, med.company, &med.price, med.category, &med.quantity, med.expiry);
        if (res != 7) {
            fputs(line, temp);
            continue;
        }
        if (med.id == id) {
            found = 1;
            if (med.quantity >= qty) {
                med.quantity -= qty;
                s.quantity = qty;
                strcpy(s.medicine, med.name);
                s.total = med.price * qty;
                sprintf(s.date, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
                fprintf(sales, "%s|%s|%d|%.2f|%s\n", s.customer, s.medicine, s.quantity, s.total, s.date);
                printf("Bill: %.2f\n", s.total);
            } else {
                printf("Not enough stock!\n");
            }
        }
        fprintf(temp, "%d|%s|%s|%.2f|%s|%d|%s\n",
            med.id, med.name, med.company, med.price, med.category, med.quantity, med.expiry);
    }

    fclose(fp);
    fclose(temp);
    fclose(sales);

    if (!found) {
        printf("Medicine Not Found!\n");
        remove("temp_medicines.txt");
    } else {
        remove("medicines.txt");
        rename("temp_medicines.txt", "medicines.txt");
    }
}

void salesReport() {
    FILE *fp = fopen("sales.txt", "r");
    if (fp == NULL) {
        printf("Error opening sales file.\n");
        return;
    }
    printf("\n --- Sales Report ---\n");
    char line[256];
    struct SaleLog s;
    while (fgets(line, sizeof(line), fp)) {
        int ok = sscanf(line, "%49[^|]|%49[^|]|%d|%f|%19[^\n]", s.customer, s.medicine, &s.quantity, &s.total, s.date);
        if (ok == 5) {
            printf("Customer: %s | Medicine: %s | Qty: %d | Total: %.2f | Date: %s\n",
                s.customer, s.medicine, s.quantity, s.total, s.date);
        }
    }
    fclose(fp);
}

void backup() {
    FILE *src, *dst;
    char line[512];

    src = fopen("medicines.txt", "r");
    dst = fopen("medicines_backup.txt", "w");
    if (src == NULL || dst == NULL) {
        printf("Error opening files for medicine backup.\n");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }
    while (fgets(line, sizeof(line), src)) fputs(line, dst);
    fclose(src);
    fclose(dst);

    src = fopen("sales.txt", "r");
    dst = fopen("sales_backup.txt", "w");
    if (src == NULL || dst == NULL) {
        printf("Error opening files for sales backup.\n");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }
    while (fgets(line, sizeof(line), src)) fputs(line, dst);
    fclose(src);
    fclose(dst);

    printf("Backup Completed Successfully.\n");
}

void restore() {
    FILE *src, *dst;
    char line[512];

    src = fopen("medicines_backup.txt", "r");
    dst = fopen("medicines.txt", "w");
    if (src == NULL || dst == NULL) {
        printf("Error opening Medicine Backup files.\n");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }
    while (fgets(line, sizeof(line), src)) fputs(line, dst);
    fclose(src);
    fclose(dst);

    src = fopen("sales_backup.txt", "r");
    dst = fopen("sales.txt", "w");
    if (src == NULL || dst == NULL) {
        printf("Error opening Sales Backup files.\n");
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }
    while (fgets(line, sizeof(line), src)) fputs(line, dst);
    fclose(src);
    fclose(dst);

    printf("Restore Completed Successfully!\n");
}

