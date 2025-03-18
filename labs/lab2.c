// Write a program that memorizes a list of geometric objects given
// as input by the user in 2D Cartesian coordinates. The objects can
// be circles, triangles and rectangles. Pack the information as much
// as possible using struct, union and bit fields and use your own
// data types. Print the list of geometric objects on the screen. Use
// the enum keyword to differentiate between the object types and
// group pairs of (x,y) coordinates together.

#include<stdio.h>
#define SHAPES_ARRAY_SIZE 120

// -------------------
// --- DEFINITIONS ---
// -------------------
typedef enum {
    CIRCLE = 1,
    RECTANGLE = 2,
    TRIANGLE = 3
} ShapeType;

typedef union {
    double x, y;
} CartesianPoint;

typedef struct {
    CartesianPoint center_point;
    double radius;

} Circle;

typedef struct {
    CartesianPoint point1, point2, point3;
} Triangle;

typedef struct {
    CartesianPoint top_left_point, bottom_right_point;
} Rectangle;

typedef struct {
    ShapeType type: 2;
    union {
        Circle circle;
        Rectangle rectangle;
        Triangle triangle;
    } u;
} GeometricShape;

// -------------
// --- MENUS ---
// -------------
void showGeneralMenu() {
    printf("\nCHOOSE YOUR SHAPE:\n1) add an object\n2) show database\n0) exit\n");
}

void showAddObjectMenu() {
    printf("What object would you like to add? (ENTER 0 FOR CANCELING THE PROCESS)\n1) circle\n2) rectangle\n3) triangle\n");
}

// ---------------
// --- ACTIONS ---
// ---------------
void showDatabase(GeometricShape shapes[], int noShapes) {
    for (int i = 0; i < noShapes; i++) {
        if (shapes[i].type == CIRCLE) {
            printf("%d) [CIRCLE] (%.2f, %.2f), r=%.2f\n", i + 1,
                   shapes[i].u.circle.center_point.x,
                   shapes[i].u.circle.center_point.y,
                   shapes[i].u.circle.radius);

        } else if (shapes[i].type == RECTANGLE) {
            printf("%d) [RECTANGLE] (%.2f, %.2f), (%.2f, %.2f)\n", i + 1,
                   shapes[i].u.rectangle.top_left_point.x,
                   shapes[i].u.rectangle.top_left_point.y,
                   shapes[i].u.rectangle.bottom_right_point.x,
                   shapes[i].u.rectangle.bottom_right_point.y);

        } else if (shapes[i].type == TRIANGLE) {
            printf("%d) [TRIANGLE] (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)\n", i + 1,
                   shapes[i].u.triangle.point1.x,
                   shapes[i].u.triangle.point1.y,
                   shapes[i].u.triangle.point2.x,
                   shapes[i].u.triangle.point2.y,
                   shapes[i].u.triangle.point3.x,
                   shapes[i].u.triangle.point3.y);

        }
    }
}

void addObjectToArray(GeometricShape shapes[], int noShapes) {
    int object_choice = 0;
    scanf("%d", &object_choice);

    if (object_choice == 1) {
        // ------------------
        // ----- CIRCLE -----
        // ------------------

        CartesianPoint center_x;
        double radius;

        shapes[noShapes].type = CIRCLE;

        printf("[CIRCLE] Enter circle data:\n");

        printf("[CIRCLE] radius = ");
        scanf("%lf", &shapes[noShapes].u.circle.radius);

        printf("[CIRCLE] center x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.circle.center_point.x);
        printf("[CIRCLE] center y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.circle.center_point.y);

    } else if (object_choice == 2) {
        // ---------------------
        // ----- RECTANGLE -----
        // ---------------------

        shapes[noShapes].type = RECTANGLE;

        printf("[RECTANGLE] Enter circle data:\n");

        printf("[RECTANGLE] top left x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.rectangle.top_left_point.x);
        printf("[RECTANGLE] top left y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.rectangle.top_left_point.y);

        printf("[RECTANGLE] bottom right x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.rectangle.bottom_right_point.x);
        printf("[RECTANGLE] bottom right y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.rectangle.bottom_right_point.y);

    } else if (object_choice == 3) {
        // --------------------
        // ----- TRIANGLE -----
        // --------------------

        shapes[noShapes].type = TRIANGLE;

        printf("[TRIANGLE] Enter circle data:\n");

        printf("[TRIANGLE] point1 x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point1.x);
        printf("[TRIANGLE] point1 y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point1.y);

        printf("[TRIANGLE] point2 x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point2.x);
        printf("[TRIANGLE] point2 y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point2.y);

        printf("[TRIANGLE] point1 x coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point3.x);
        printf("[TRIANGLE] point1 y coordinate = ");
        scanf("%lf", &shapes[noShapes].u.triangle.point3.y);
    }
}


int main() {

    GeometricShape shapes[SHAPES_ARRAY_SIZE];
    int k = 0;

    // ----- reading the options -----
    int current_option = -1;
    while (current_option != 0) {

        showGeneralMenu();
        scanf("%d", &current_option);

        if (current_option == 1) {
            // ----- adding an object scenario -----
            printf("\n[ACTION] Adding an object...\n");

            showAddObjectMenu();
            addObjectToArray(shapes, k);
            k++;

        } else if (current_option == 2) {
            // ----- showing database scenario -----
            printf("\n[ACTION] Showing the database...\n");
            showDatabase(shapes, k);

        } else if (current_option == 0) {
            // ----- exiting scenario -----
            printf("\n[ACTION] Exiting the program...\n");
        }
    }



    return 0;
}