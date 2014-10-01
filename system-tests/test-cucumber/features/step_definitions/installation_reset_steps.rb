require '../helpers/qa_backdoor.rb'

Given /^a fresh gardens installation$/ do
  raise "Can't reset gardens install. Current capapilities: #{$site_capabilities}" unless $site_capabilities[:backdoor]
  backdoor = QaBackdoor.new($config['sut_url'], { :logger => $logger })
  backdoor.restore_snapshot
  step "I have a new session"
end
