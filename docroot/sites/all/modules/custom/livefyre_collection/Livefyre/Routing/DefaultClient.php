<?php

namespace Livefyre\Routing;


use Livefyre\Exceptions\ApiException;
use Requests;
/**
 * @codeCoverageIgnore
 */
class DefaultClient {
    public static function get($url, $headers) {
        $response = Requests::get($url, $headers);

        self::examineResponse($response->status_code);
        return $response->body;
    }

    public static function post($url, $headers, $data) {
        $response = Requests::post($url, $headers, $data);

        self::examineResponse($response->status_code);
        return $response->body;
    }

    public static function put($url, $headers, $data) {
        $response = Requests::put($url, $headers, $data);

        self::examineResponse($response->status_code);
        return $response->body;
    }

    public static function patch($url, $headers, $data) {
        $response = Requests::patch($url, $headers, $data);

        self::examineResponse($response->status_code);
        return $response->body;
    }

    private static function examineResponse($code) {
        if ($code >= 400) {
            throw new ApiException($code);
        }
    }
}