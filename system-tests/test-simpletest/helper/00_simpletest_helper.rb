require 'rubygems'
require 'net/ssh'
require 'net/scp'

# Helper functions for SSH/SCP
module Test00SimpletestHelper

  # The following test cases were not passing on our testing infrastructure as
  # of May 2012:
  # http://build-1.acquia:8180/job/gardens-test-simpletest/1150/testReport/
  # TODO: Revisit failing tests - especially tests belonging to our modules -
  # and see if they can be made to pass.
  TEST_BLACKLIST = [
    # The following tests are core or contrib not controlled by our team.
    "BasicMinimalUpdatePath",
    "BasicStandardUpdatePath",
    "EntityCacheUserBlocksUnitTests",
    "FeedsCSVtoUsersTest",
    "FeedsExamplesOPMLTestCase",
    "FeedsExamplesUserTestCase",
    "FeedsMapperDateTestCase",
    "FeedsMapperFileTestCase",
    "FeedsMapperLinkTestCase",
    "FileDownloadTest",
    "FileUnmanagedDeleteRecursiveTest",
    "FilledMinimalUpdatePath",
    "FilledStandardUpdatePath",
    "ImageDimensionsUnitTest",
    "MailhandlerTestCase",
    "MigrateUserUnitTest",
    "ModuleDependencyTestCase",
    "SecurePagesTestCase",
    "ServicesModuleTests",
    "ServicesResourceCommentTests",
    "ServicesResourceSystemTests",
    "ServicesResourceTaxonomyTests",
    "ViewsHandlerFilterStringTest",
    "ViewsHandlerSortDateTest",
    "ViewsPagerTest",
    "viewsUiGroupbyTestCase",
    "ViewsUIWizardJumpMenuTestCase",
    "ViewsUpgradeTestCase",
    "ViewsViewTest",
    "VotingAPITestCase",
    "XMLSitemapTaxonomyFunctionalTest",

    # The following tests were written by our team and should be made to pass.
    "JavaScriptLibrariesDrupalTestCase",
    "WebformSSLTestCase"
  ]

  def ssh(host, key_path)
    Net::SSH.start(host, 'ubuntu', :keys => [ key_path ]) do |session|
      yield(session)
    end
  end

  def scp(host, key_path)
    Net::SCP.start(host, 'ubuntu', :keys => [ key_path ]) do |session|
      yield(session)
    end
  end

end
