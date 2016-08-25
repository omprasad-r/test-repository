<?php

namespace Livefyre\Routing;


use Livefyre\Pattern\Singleton;
/**
 * @codeCoverageIgnore
 */
class ClientFactory extends Singleton {
    const DEFAULT_CLIENT = 'Livefyre\Routing\DefaultClient';
    const WP_CLIENT = 'Livefyre\Routing\WPClient';

    private $clientType = self::DEFAULT_CLIENT;
    private $customType = False;

    public function get($url, $headers) {
        $client = $this->clientType;
        return $client::get($url, $headers);
    }

    public function post($url, $headers, $data, $handle) {
        $client = $this->clientType;
        return $client::post($url, $headers, $data, $handle);
    }

    public function put($url, $headers, $data) {
        $client = $this->clientType;
        return $client::put($url, $headers, $data);
    }

    public function patch($url, $headers, $data) {
        $client = $this->clientType;
        return $client::patch($url, $headers, $data);
    }

    public function setToDefault() {
        $this->clientType = self::DEFAULT_CLIENT;
    }

    public function setToWP() {
        $this->clientType = self::WP_CLIENT;
    }

    public function getType() {
        return $this->clientType;
    }

    public function setType($newType) {
        $this->customType = True;
        $this->clientType = $newType;
    }

    public function isCustom() {
        return $this->customType;
    }
}