<?php

/**
 * Implements class RedemptionCouponController
 */
class RedemptionCouponController extends DrupalDefaultEntityController {
  public function save($coupon) {
    if (isset($coupon->cid) && !empty($coupon->cid)) {
      drupal_write_record('redemption_coupon', $coupon, 'cid');
      field_attach_update('coupon', $coupon);
      module_invoke_all('entity_update', $coupon, 'coupon');
    }
    else {
      unset($coupon->cid);
      drupal_write_record('redemption_coupon', $coupon);
      field_attach_insert('coupon', $coupon);
      module_invoke_all('entity_insert', $coupon, 'coupon');
      return $coupon;
    }
  }
}

/**
 * Implements class RedemptionCouponUIController
 */
class RedemptionCouponUIController extends EntityDefaultUIController {
  /**
   * Generates the row for the passed entity and may be overridden in order to
   * customize the rows.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $cols[] = array(
      'data' => array(
        '#theme' => 'entity_ui_overview_item',
        '#label' => $entity->code,
        '#name' => !empty($this->entityInfo['exportable']) ? entity_id($this->entityType, $entity) : FALSE,
        '#url' => '',
        '#entity_type' => $this->entityType
      ),
    );
    $cols[] = array(
      'data' => array(
        '#theme' => 'entity_status',
        '#status' => $entity->status,
      ),
    );
    $cols[] = l(t('edit'), $this->path . '/edit/' . $id);
    $cols[] = l(t('delete'), $this->path . '/delete/' . $id);
    return $cols;
  }
}
