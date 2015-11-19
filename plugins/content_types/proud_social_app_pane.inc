<?php

$plugin = array(
  'single' => TRUE,
  'title' => t('Social feed'),
  'description' => t('Dynamic social media feed'),
  'category' => t('Blocks'),
  'edit form' => 'proud_social_app_pane_edit_form',
  'render callback' => 'proud_social_app_render_pane',
  'defaults' => array(),
  'all contexts' => TRUE,
);


function proud_social_app_pane_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['accounts'] = array(
    '#type' => 'radios',
    '#title' => t('Social accounts'),
    '#default_value' => !empty($conf['accounts']) ? $conf['accounts'] : 'all',
    '#options' => array(
      'all' => t('All (city-wide)'),
      'agency' => t('Accounts belonging to agency'),
      'custom' => t('Specific accounts')
    )
  );


  // Quick and dirty: Create $options of all Agency nodes
  // From https://www.drupal.org/node/1943636
  $options = array('' => t('- Select -'));
  $query = db_select('node', 'n');
  $result = $query
    ->condition('n.type', 'agency', 'like')
    ->condition('n.status', 1)
    ->fields('n', array('nid', 'title'))
    ->execute();
  foreach ($result as $n) {
    $options[_proud_social_app_sanitize_key($n->title)] = $n->title;
  } 
  $form['agency'] = array(
    '#type' => 'select',
    '#title' => t('Agency'),
    '#default_value' => !empty($conf['agency']) ? $conf['agency'] : '',
    '#options' => $options,
    '#states' => array(
      'visible' => array(
        ':input[name="accounts"]' => array('value' => 'agency'),
      ),
    ),
  );


  // @todo: we need to loop through these and validate that we are watching with the social app (call to /api/info)
  $form['custom'] = array(
    '#type' => 'textarea',
    '#rows' => 3,
    '#title' => t('Accounts to display'),
    '#description' => t("<p>Enter one per line.  Example:</p><p><code>twitter|whitehouse<br/>instagram|usinterior</code></p><p>!link</p>",array(
      '!link' => l('See available social accounts', 'admin/content/social', array('target', '_blank'))
    )),
    '#default_value' => !empty($conf['custom']) ? $conf['custom'] : '',
    '#options' => $options,
    '#states' => array(
      'visible' => array(
        ':input[name="accounts"]' => array('value' => 'custom'),
      ),
    ),
  );


  $services = variable_get('socialfield_services', array());
  $options = array();
  foreach($services as $key => $service) {
    $options[$key] = $service['name'];
  }
  $form['services'] = array(
    '#type' => 'checkboxes',
    '#multiple' => TRUE,
    '#title' => t('Services'),
    '#default_value' => !empty($conf['services']) ? $conf['services'] : array_keys($options),
    '#options' => $options,
    '#states' => array(
      'hidden' => array(
        ':input[name="accounts"]' => array('value' => 'custom'),
      ),
    ),
  );


  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['display']['widget_type'] = array(
    '#type' => 'radios',
    '#title' => t('Widget type'),
    //'#description' => t('Choose between a social wall, and a timeline.'),
    '#default_value' => !empty($conf['widget_type']) ? $conf['widget_type'] : 'feed',
    '#options' => array(
      'feed' => t('Feed'),
      'wall' => t('Social wall'),
      'timeline' => t('Timeline')
    )
  );

  $form['display']['show_controls'] = array(
    '#type' => 'radios',
    '#title' => t('Show filters?'),
    '#description' => t('Show controls above the widget'),
    '#default_value' => !empty($conf['show_controls']) ? $conf['show_controls'] : 'true',
    '#options' => array(
      'false' => t('No'),
      'true' => t('Yes')
    )
  );

  $form['display']['post_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Count'),
    '#description' => t('How many items to show'),
    '#default_value' => !empty($conf['post_count']) ? $conf['post_count'] : '20',
    '#size' => 3, 
  );

  return $form;
}

/**
 * Edit form submit function.
 */
function proud_social_app_pane_edit_form_submit($form, &$form_state) {
  $form_state['conf']['accounts'] = $form_state['values']['accounts'];
  $form_state['conf']['agency'] = $form_state['values']['agency'];
  $form_state['conf']['custom'] = $form_state['values']['custom'];
  $form_state['conf']['services'] = $form_state['values']['services'];
  $form_state['conf']['widget_type'] = $form_state['values']['widget_type'];
  $form_state['conf']['show_controls'] = $form_state['values']['show_controls'];
  $form_state['conf']['post_count'] = $form_state['values']['post_count'];
}