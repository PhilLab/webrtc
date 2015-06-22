#include "common.h"

namespace BoringSSLTests
{

  //test entry point declaration
  extern "C"
  {
    int boringSSL_base64_test_main(void);
    int boringSSL_bio_test_main(void);
    int boringSSL_bn_test_main_wrapped(void);
    int boringSSL_bytestring_test_main(void);
    int boringSSL_constant_time_test_main(void);
    int boringSSL_dh_test_main(void);
    int boringSSL_digest_test_main(void);
    int boringSSL_dsa_test_main(void);
    int boringSSL_ec_test_main(void);
    int boringSSL_ecdsa_test_main(void);
    int boringSSL_err_test_main(void);
    int boringSSL_gcm_test_main(void);
    int boringSSL_hmac_test_main(void);
    int boringSSL_lhash_test_main(void);
    int boringSSL_rsa_test_main(void);
    int boringSSL_pkcs7_test_main(void);
    int boringSSL_pkcs12_test_main(void);
    int boringSSL_example_mul_test_main(void);
    int boringSSL_evp_test_main(void);
    int boringSSL_ssl_test_main(void);
    int boringSSL_pqueue_test_main(void);
    int boringSSL_hkdf_test_main(void);
    int boringSSL_pbkdf_test_main(void);
    int boringSSL_thread_test_main(void);
  }

  // Simple test table
  static const SimpleTestDescT SimpleTests[] = {
    { L"base64_test", L"boringssl_base64_test", boringSSL_base64_test_main },
    { L"bio_test", L"boringssl_bio_test", boringSSL_bio_test_main },
    { L"bn_test", L"boringssl_bn_test", boringSSL_bn_test_main_wrapped },
    { L"bytestring_test", L"boringssl_bytestring_test", boringSSL_bytestring_test_main },
    { L"constant_time_test", L"boringssl_constant_time_test", boringSSL_constant_time_test_main },
    { L"dh_test", L"boringssl_dh_test", boringSSL_dh_test_main },
    { L"digest_test", L"boringssl_digest_test", boringSSL_digest_test_main },
    { L"dsa_test", L"boringssl_dsa_test", boringSSL_dsa_test_main },
    { L"ec_test", L"boringssl_ec_test", boringSSL_ec_test_main },
    { L"ecdsa_test", L"boringssl_ecdsa_test", boringSSL_ecdsa_test_main },
    { L"err_test", L"boringssl_err_test", boringSSL_err_test_main },
    { L"gcm_test", L"boringssl_gcm_test", boringSSL_gcm_test_main },
    { L"hmac_test", L"boringssl_hmac_test", boringSSL_hmac_test_main },
    { L"lhash_test", L"boringssl_lhash_test", boringSSL_lhash_test_main },
    { L"rsa_test", L"boringssl_rsa_test", boringSSL_rsa_test_main },
    { L"pkcs7_test", L"boringssl_pkcs7_test", boringSSL_pkcs7_test_main },
    { L"pkcs12_test", L"boringssl_pkcs12_test", boringSSL_pkcs12_test_main },
    { L"example_mul_test", L"boringssl_example_mul_test", boringSSL_example_mul_test_main },
    { L"evp_test", L"boringssl_evp_test", boringSSL_evp_test_main },
    { L"ssl_test", L"boringssl_ssl_test", boringSSL_ssl_test_main },
    { L"pqueue_test", L"boringssl_pqueue_test", boringSSL_pqueue_test_main },
    { L"hkdf_test", L"boringssl_hkdf_test", boringSSL_hkdf_test_main },
    { L"pbkdf_test", L"boringssl_pbkdf_test", boringSSL_pbkdf_test_main },
    { L"thread_test", L"boringssl_thread_test", boringSSL_thread_test_main },
    { NULL, NULL, NULL },
  };

  //boringSLL bn_test uses different type of main function, so it's wrapped
  extern "C" int boringSSL_bn_test_main(int argc, char *argv[]);
  int boringSSL_bn_test_main_wrapped(void)
  {
    char* argv[] = { "." };
    return boringSSL_bn_test_main(1, argv);
  }

  // Automatic test inserter 
  BoringSSLSimpleTestInserter<SingleInstanceTestSolutionProvider> CBoringSSLSimpleTest::_inserter(SimpleTests);

  int CBoringSSLSimpleTest::InterchangeableExecute()
  {
    if (m_pTestFunction != NULL)
    {
      return m_pTestFunction();
    }

    return 0;
  }
  
  CBoringSSLSimpleTest::CBoringSSLSimpleTest(const SimpleTestDescT& desc)
    : m_Name(desc.pName)
    , m_Project(desc.pProject)
    , m_pTestFunction(desc.pFunction)
  {
    
  }

}

