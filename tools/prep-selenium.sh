#!/bin/bash
if [ "$EUID" -ne 0 ]
  then
  echo 'You need to run this via sudo to succeed'
  exit 1
fi

echo updating gems
gem update --system

echo installing required gems
gem install activeresource  \
          activesupport \
          capybara \
          capybara-mechanize \
          mocha \
          net-netrc \
          net-sftp \
          net-ssh \
          net-ssh-gateway \
          ruby-debug \
          ruby-debug-ide \
          soap4r \
          sources \
          Selenium \
          selenium-client \
          right_aws \
          net-scp \
          relative \
          faker \
          configtoolkit

echo Getting selenium jar
TMPDIR=/tmp
CURRENT_VERSION=2.3.0
pushd $TMPDIR
rm -f selenium-server-standalone-*.jar
curl -o $TMPDIR/selenium-server-standalone-$CURRENT_VERSION.jar http://selenium.googlecode.com/files/selenium-server-standalone-$CURRENT_VERSION.jar
popd
INSTALL_DIR=/usr/local/lib/selenium
echo Installing jar into $INSTALL_DIR
mkdir -p $INSTALL_DIR
cp $TMPDIR/selenium-server-standalone-$CURRENT_VERSION.jar $INSTALL_DIR/.
echo Building selenium script
echo "#!/bin/bash" > /usr/local/bin/selenium
echo "java -jar $INSTALL_DIR/selenium-server-standalone-$CURRENT_VERSION.jar $@" >> /usr/local/bin/selenium
chmod +x /usr/local/bin/selenium
echo Selenium is installed use /usr/local/bin/selenium to start a selenium server


