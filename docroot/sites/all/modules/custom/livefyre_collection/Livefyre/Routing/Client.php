<?php

namespace Livefyre\Routing;


/**
 * @codeCoverageIgnore
 */
class Client {
	public static function GET($url, $headers = array()) {
        return self::getClient()->get($url, $headers);
	}

	public static function POST($url, $headers = array(), $data = array(), $handle = true) {
        return self::getClient()->post($url, $headers, $data, $handle);
	}

	public static function PUT($url, $headers = array(), $data = array()) {
        return self::getClient()->put($url, $headers, $data);
	}

	public static function PATCH($url, $headers = array(), $data = array()) {
        return self::getClient()->patch($url, $headers, $data);
	}

    private static function getClient() {
        $client = ClientFactory::getInstance();

        if (!$client->isCustom() && (function_exists("wp_remote_get") && $client->getType() != ClientFactory::WP_CLIENT)) {
            $client->setToWP();
        }

        return $client;
    }
}