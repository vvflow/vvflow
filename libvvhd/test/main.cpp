#include "test_TVec.hpp"
#include "test_TVec3D.hpp"
#include "test_TEval.hpp"
#include "test_TBody.hpp"
#include "test_XField.hpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
