<?php

namespace api\modules\v1;

use yii\filters\ContentNegotiator;
use yii\web\Response;

/**
 * Class Controller
 * @package api\modules\v1
 */
class Controller extends \yii\rest\Controller
{
    /**
     * {@inheritdoc}
     */
    public function behaviors()
    {
        return array_merge(parent::behaviors(), [
            'contentNegotiator' => [
                'class' => ContentNegotiator::class,
                'formats' => [
                    'application/json' => Response::FORMAT_RAW,
                    'application/xml' => Response::FORMAT_RAW,
                ],
            ],
        ]);
    }
}
