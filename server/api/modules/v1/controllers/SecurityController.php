<?php

namespace api\modules\v1\controllers;

use api\modules\v1\Controller;

/**
 * @package api\modules\v1\controllers
 */
class SecurityController extends Controller
{
    const STATUS_UNLOCKED = 1;
    const STATUS_LOCKED = 2;

    /**
     * @const время, через которое нужно сбросить данные датчиков.
     */
    const TIME_SENSOR_RESET = 20;

    static $sensors = [
        'rswitch',
        'shock',
        'voltage',
        'signal',
    ];

    /**
     * Вернет статус безопасности - на охране объект или нет.
     * @param bool $isApp true, если запрашивает приложение.
     * @return string
     */
    public function actionGetStatus(bool $isApp = false)
    {
        $redis = \Yii::$app->redis;
        $req = \Yii::$app->request;
        if (!$isApp) {
            //  Запоминаем дату последнего запроса модуля.
            $redis->set('security:lastReq', time());

            //  Запомним показания с датчиков.
            foreach (self::$sensors as $sensor) {
                $redis->set('sensor:' . $sensor, $req->get($sensor, -1));
            }
        }

        //  Сбросываем данные с датчиков.
        if ($redis->get('security:lastReq') + self::TIME_SENSOR_RESET < time()) {
            foreach (self::$sensors as $sensor) {
                $redis->set('sensor:' . $sensor, 0);
            }
        }

        return $redis->get('security:status') ?? '0';
    }

    /**
     * Вернет дату последнего обращения модуля.
     * @return mixed
     */
    public function actionGetLastReq()
    {
        return \Yii::$app->redis->get('security:lastReq');
    }

    /**
     * Вернет состояние сенсоров модуля.
     * @return mixed
     */
    public function actionGetStateSensors()
    {
        $r = \Yii::$app->redis;
        return implode(':', [
            $r->get('sensor:rswitch') ?? '0',
            $r->get('sensor:shock') ?? '0',
            $r->get('sensor:voltage') ?? '0',
            $r->get('sensor:signal') ?? '0',
        ]);
    }

    /**
     * Установит текущий статус.
     */
    public function actionSetStatus($status)
    {
        //  Если разблокируем, то стираем дату последнего запроса модуля.
        if ($status == self::STATUS_UNLOCKED) {
            \Yii::$app->redis->del('security:lastReq');
        }

        return \Yii::$app->redis->set('security:status', $status == self::STATUS_UNLOCKED ? self::STATUS_UNLOCKED : self::STATUS_LOCKED);
    }
}
