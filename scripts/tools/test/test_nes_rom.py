def test_commercial_rom():
    """Testa uma ROM comercial do NES"""
    test_config = {
        'rom_path': 'test_roms/commercial.nes',
        'expected_crc': 0x12345678,
        'test_duration': 30,  # segundos
        'check_points': [
            {'frame': 1, 'test': 'title_screen'},
            {'frame': 60, 'test': 'gameplay'},
            {'frame': 120, 'test': 'sprite_render'}
        ]
    }
    
    results = run_nes_test(test_config)
    generate_test_report(results)
    return results