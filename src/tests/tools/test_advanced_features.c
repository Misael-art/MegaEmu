void test_dashboard_configuration(void) {    Dashboard* dash = dashboard_create();    Widget test_widget = create_test_widget();        dash->add_widget(&test_widget);    dash->save_layout("test_layout.json");        Dashboard* loaded = dashboard_create();    loaded->load_layout("test_layout.json");        TEST_ASSERT_EQUAL(1, loaded->widget_count);    TEST_ASSERT_EQUAL_STRING(test_widget.widget_id, loaded->widgets[0]->widget_id);        dashboard_destroy(dash);    dashboard_destroy(loaded);}void test_plugin_system(void) {    PluginManager* pm = plugin_manager_create();    VisualizationPlugin* test_plugin = create_test_plugin();        pm->register_plugin(test_plugin);        TEST_ASSERT_EQUAL(1, pm->plugin_count);    TEST_ASSERT_NOT_NULL(pm->plugins[0]->render_frame);        plugin_manager_destroy(pm);}void test_ml_alerts(void) {    MLAlertSystem* ml = ml_alert_system_create();    MetricsData test_data = generate_test_metrics();        ml->process_metrics(&test_data);        TEST_ASSERT_NOT_NULL(ml->predictor->model);    TEST_ASSERT_TRUE(ml->metrics.accuracy > 0.9);        ml_alert_system_destroy(ml);}void test_storage_optimization(void) {    MetricsStore* store = metrics_store_create(&test_config);    MetricsData test_data = generate_large_test_dataset();        store->store_metrics(&test_data);    store->optimize_database();        float compression_ratio = store->optimizer->compression_ratio;    TEST_ASSERT_TRUE(compression_ratio > 2.0);        metrics_store_destroy(store);}