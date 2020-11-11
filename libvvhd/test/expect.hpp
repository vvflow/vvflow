#define EXPECT_THROW_MSG(statement, expected_exception, expected_what)         \
  do {                                                                         \
    bool cpputCorrectExceptionThrown_ = false;                                 \
    CPPUNIT_NS::Message cpputMsg_( "expected exception not thrown" );          \
    cpputMsg_.addDetail( CPPUNIT_NS::AdditionalMessage() );                    \
    cpputMsg_.addDetail( "Expected: "                                          \
                         CPPUNIT_GET_PARAMETER_STRING( expected_exception ) ); \
                                                                               \
    try {                                                                      \
      statement;                                                               \
    } catch ( const expected_exception &e ) {                                  \
      CPPUNIT_ASSERT_EQUAL(std::string(expected_what), std::string(e.what())); \
      cpputCorrectExceptionThrown_ = true;                                     \
    } catch ( const std::exception &e) {                                       \
      cpputMsg_.addDetail( "Actual  : " +                                      \
                           CPPUNIT_EXTRACT_EXCEPTION_TYPE_( e,                 \
                                       "std::exception or derived") );         \
      cpputMsg_.addDetail( std::string("What()  : ") + e.what() );             \
    } catch ( ... ) {                                                          \
      cpputMsg_.addDetail( "Actual  : unknown.");                              \
    }                                                                          \
                                                                               \
    if ( cpputCorrectExceptionThrown_ )                                        \
      break;                                                                   \
                                                                               \
    CPPUNIT_NS::Asserter::fail( cpputMsg_,                                     \
                                CPPUNIT_SOURCELINE() );                        \
 } while ( false )


#define EXPECT_EQUAL(x, y)                                                     \
  if (!std::isnan(x) || !std::isnan(y)) {                                      \
      CPPUNIT_ASSERT_EQUAL(x, y);                                              \
  }
